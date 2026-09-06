#include "imgui_impl_rendergraph.h"
#include <Render/RenderGraph/RenderGraph.h>
#include <Render/Upload/PendingUploadList.h>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace Aether;
namespace RG = Aether::RenderGraph;

namespace
{
struct TextureBinding
{
    rhi::Texture2D ownedTexture;
    rhi::TextureView ownedView;
    rhi::Texture2D* texture = nullptr;
    rhi::TextureView* view = nullptr;
    std::optional<vk::DescriptorPool> pool;
    std::optional<vk::DescriptorSet> set;
};

struct Backend
{
    PixelFormat colorFormat;
    rhi::Sampler sampler;
    std::optional<vk::DescriptorSetLayout> descriptorLayout;
    std::optional<vk::PipelineLayout> layout;
    rhi::Pipeline pipeline;
    std::unordered_map<ImTextureID, std::unique_ptr<TextureBinding>> textures;
    struct RetiredTexture
    {
        std::unique_ptr<TextureBinding> binding;
        int framesLeft;
    };
    std::vector<RetiredTexture> retiredTextures;
    ImTextureID nextTexture = 1;
};

Backend& GetBackend()
{
    auto* backend = static_cast<Backend*>(ImGui::GetIO().BackendRendererUserData);
    IM_ASSERT(backend && "ImGui RenderGraph backend is not initialized");
    return *backend;
}

void CreateBinding(Backend& backend, TextureBinding& binding, rhi::Sampler& sampler)
{
    binding.pool = vk::DescriptorPool::Builder().PushSampler(1).MaxSets(1).Build();
    if (!binding.pool)
        throw std::runtime_error("ImGui: failed to allocate descriptor pool");
    binding.set = vk::DescriptorSet::Create(*backend.descriptorLayout, *binding.pool);
    if (!binding.set)
        throw std::runtime_error("ImGui: failed to allocate texture descriptor");
    vk::DescriptorSetOperator op(*binding.set);
    op.BindSampler(0, sampler.GetVk(), binding.view->GetVk());
    op.Apply();
}

constexpr const char* VertexCode = R"(
#version 450
layout(location=0) in vec2 position;
layout(location=1) in vec2 uv;
layout(location=2) in uint color;
layout(push_constant) uniform Transform { vec2 scale; vec2 translate; } transform;
layout(location=0) out vec2 outUV;
layout(location=1) out vec4 outColor;
void main() {
    outUV = uv;
    outColor = unpackUnorm4x8(color);
#ifdef IMGUI_COLOR_BGRA
    outColor = outColor.bgra;
#endif
    gl_Position = vec4(position * transform.scale + transform.translate, 0.0, 1.0);
}
)";
constexpr const char* PixelCode = R"(
#version 450
layout(set=0,binding=0) uniform sampler2D image;
layout(location=0) in vec2 uv;
layout(location=1) in vec4 color;
layout(location=0) out vec4 outColor;
void main() { outColor = color * texture(image, uv); }
)";

struct DrawTask
{
    Backend* backend = nullptr;
    ImDrawData* drawData = nullptr;
    RG::AccessId<rhi::VertexBuffer> vertices;
    RG::AccessId<rhi::IndexBuffer> indices;
    uint32_t width = 0, height = 0;
};

void Draw(rhi::CommandList& commands, RG::ResourceAccessor& resources, DrawTask& task)
{
    ImDrawData& data = *task.drawData;
    if (data.TotalVtxCount <= 0 || data.TotalIdxCount <= 0)
        return;
    auto& backend = *task.backend;
    auto& vertices = resources.GetResource(task.vertices)->GetVk();
    auto& indices = resources.GetResource(task.indices)->GetVk();
    size_t vertexOffset = 0, indexOffset = 0;
    for (const ImDrawList* list : data.CmdLists)
    {
        const size_t vertexBytes = list->VtxBuffer.Size * sizeof(ImDrawVert);
        const size_t indexBytes = list->IdxBuffer.Size * sizeof(ImDrawIdx);
        vertices.SetData(vertexOffset, {reinterpret_cast<const uint8_t*>(list->VtxBuffer.Data), vertexBytes});
        indices.SetData(indexOffset, {reinterpret_cast<const uint8_t*>(list->IdxBuffer.Data), indexBytes});
        vertexOffset += vertexBytes;
        indexOffset += indexBytes;
    }

    // These bindings are not exposed by CommandList yet. Resource ownership,
    // attachment setup and layout transitions remain with the RenderGraph.
    VkCommandBuffer cb = commands.GetVk().GetHandle();
    auto setup = [&]() {
        commands.BindPipeline(backend.pipeline);
        commands.SetViewport(0, 0, static_cast<float>(task.width), static_cast<float>(task.height));
        commands.SetScissor(0, 0, static_cast<float>(task.width), static_cast<float>(task.height));
        VkBuffer vertexBuffer = vertices.GetHandle();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(cb, indices.GetHandle(), 0,
                            sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        float transform[4] = {2.0f / data.DisplaySize.x, 2.0f / data.DisplaySize.y, 0, 0};
        transform[2] = -1.0f - data.DisplayPos.x * transform[0];
        transform[3] = -1.0f - data.DisplayPos.y * transform[1];
        vkCmdPushConstants(cb, backend.layout->GetHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(transform), transform);
    };
    setup();
    ImGui_ImplRenderGraph_RenderState state{&commands, &backend.pipeline};
    void* previousState = ImGui::GetPlatformIO().Renderer_RenderState;
    ImGui::GetPlatformIO().Renderer_RenderState = &state;
    int baseVertex = 0;
    uint32_t baseIndex = 0;
    for (const ImDrawList* list : data.CmdLists)
    {
        for (const ImDrawCmd& command : list->CmdBuffer)
        {
            if (command.UserCallback)
            {
                if (command.UserCallback == ImDrawCallback_ResetRenderState)
                    setup();
                else
                    command.UserCallback(list, &command);
                continue;
            }
            if (command.ElemCount == 0)
                continue;
            float x1 = (command.ClipRect.x - data.DisplayPos.x) * data.FramebufferScale.x;
            float y1 = (command.ClipRect.y - data.DisplayPos.y) * data.FramebufferScale.y;
            float x2 = (command.ClipRect.z - data.DisplayPos.x) * data.FramebufferScale.x;
            float y2 = (command.ClipRect.w - data.DisplayPos.y) * data.FramebufferScale.y;
            x1 = std::clamp(x1, 0.0f, static_cast<float>(task.width));
            y1 = std::clamp(y1, 0.0f, static_cast<float>(task.height));
            x2 = std::clamp(x2, 0.0f, static_cast<float>(task.width));
            y2 = std::clamp(y2, 0.0f, static_cast<float>(task.height));
            if (x2 <= x1 || y2 <= y1)
                continue;
            auto binding = backend.textures.find(command.GetTexID());
            IM_ASSERT(binding != backend.textures.end() && "Texture must be registered with RenderGraph backend");
            if (binding == backend.textures.end())
                continue;
            commands.SetScissor(x1, y1, x2 - x1, y2 - y1);
            VkDescriptorSet set = binding->second->set->GetHandle();
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, backend.layout->GetHandle(),
                                    0, 1, &set, 0, nullptr);
            vkCmdDrawIndexed(cb, command.ElemCount, 1, baseIndex + command.IdxOffset,
                             baseVertex + static_cast<int>(command.VtxOffset), 0);
        }
        baseVertex += list->VtxBuffer.Size;
        baseIndex += list->IdxBuffer.Size;
    }
    ImGui::GetPlatformIO().Renderer_RenderState = previousState;
    commands.SetScissor(0, 0, static_cast<float>(task.width), static_cast<float>(task.height));
}
} // namespace

bool ImGui_ImplRenderGraph_Init(const ImGui_ImplRenderGraph_InitInfo& info)
{
    auto& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr);
    auto backend = std::make_unique<Backend>();
    backend->colorFormat = info.ColorFormat;
    auto sampler = vk::Sampler::Builder().SetDefaultCreateInfo()
        .SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE).SetAnisotropyEnable(false).Build();
    if (!sampler)
        return false;
    backend->sampler = rhi::Sampler(std::move(*sampler));
    backend->descriptorLayout = vk::DescriptorSetLayout::Builder()
        .BeginSamplerBinding().UseFragmentStage().EndSamplerBinding().Build();
    if (!backend->descriptorLayout)
        return false;
    backend->layout = vk::PipelineLayout::Builder().AddDescriptorSetLayout(*backend->descriptorLayout)
        .AddPushConstantRange(4 * sizeof(float), static_cast<vk::ShaderStageFlags>(VK_SHADER_STAGE_VERTEX_BIT)).Build();
    if (!backend->layout)
        return false;
    std::string preamble;
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
    preamble = "#define IMGUI_COLOR_BGRA\n";
#endif
    auto vertex = rhi::VertexShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, VertexCode, preamble));
    auto pixel = rhi::PixelShader::Create(ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, PixelCode));
    if (!vertex || !pixel)
        return false;
    VkVertexInputBindingDescription binding{0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributes[] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)},
        {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)},
        {2, 0, VK_FORMAT_R32_UINT, offsetof(ImDrawVert, col)},
    };
    VkFormat format = PixelFormatToVkFormat(info.ColorFormat);
    auto pipeline = vk::GraphicsPipeline::Builder(*backend->layout)
        .SetDynamicRenderingFormats({&format, 1})
        .PushVertexInputLayout({&binding, 1}, attributes)
        .AddVertexStage(vertex->GetVk(), "main").AddFragmentStage(pixel->GetVk(), "main")
        .BeginColorAttachment().EnableBlend(true).EndColorAttachment().Build();
    if (!pipeline)
        return false;
    backend->pipeline = rhi::Pipeline(std::move(*pipeline));
    io.BackendRendererUserData = backend.release();
    io.BackendRendererName = "imgui_impl_rendergraph";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
    return true;
}

void ImGui_ImplRenderGraph_NewFrame()
{
    (void)GetBackend();
}

ImTextureID ImGui_ImplRenderGraph_AddTexture(rhi::Texture2D& texture, rhi::TextureView& view, rhi::Sampler& sampler)
{
    auto& backend = GetBackend();
    auto binding = std::make_unique<TextureBinding>();
    binding->texture = &texture;
    binding->view = &view;
    CreateBinding(backend, *binding, sampler);
    const ImTextureID id = backend.nextTexture++;
    backend.textures.emplace(id, std::move(binding));
    return id;
}

void ImGui_ImplRenderGraph_RemoveTexture(ImTextureID texture)
{
    GetBackend().textures.erase(texture);
}

void ImGui_ImplRenderGraph_UpdateTextures(ImDrawData* data, PendingUploadList& uploads)
{
    auto& backend = GetBackend();
    std::erase_if(backend.retiredTextures, [](Backend::RetiredTexture& texture) {
        return --texture.framesLeft <= 0;
    });
    if (!data || !data->Textures)
        return;
    for (ImTextureData* texture : *data->Textures)
    {
        if (texture->Status == ImTextureStatus_WantDestroy)
        {
            auto found = backend.textures.find(texture->GetTexID());
            if (found != backend.textures.end())
            {
                backend.retiredTextures.push_back({std::move(found->second), Render::Config::MaxFramesInFlight});
                backend.textures.erase(found);
            }
            texture->SetTexID(ImTextureID_Invalid);
            texture->BackendUserData = nullptr;
            texture->SetStatus(ImTextureStatus_Destroyed);
        }
        else if (texture->Status == ImTextureStatus_WantCreate || texture->Status == ImTextureStatus_WantUpdates)
        {
            IM_ASSERT(texture->Format == ImTextureFormat_RGBA32);
            const bool create = texture->Status == ImTextureStatus_WantCreate;
            std::unique_ptr<TextureBinding> newBinding;
            TextureBinding* binding;
            if (create)
            {
                newBinding = std::make_unique<TextureBinding>();
                binding = newBinding.get();
                rhi::TextureDesc desc{};
                desc.width = texture->Width;
                desc.height = texture->Height;
                desc.pixelFormat = PixelFormat::RGBA8888;
                desc.usages = PackFlags(rhi::TextureUsage::Sample, rhi::TextureUsage::TransferDst);
                desc.layout = rhi::TextureLayout::Undefined;
                binding->ownedTexture = rhi::Texture2D::Create(desc);
                if (!binding->ownedTexture)
                    throw std::runtime_error("ImGui: failed to create font texture");
                binding->ownedView = binding->ownedTexture.CreateImageView({});
                binding->texture = &binding->ownedTexture;
                binding->view = &binding->ownedView;
                CreateBinding(backend, *binding, backend.sampler);
            }
            else
            {
                binding = backend.textures.at(texture->GetTexID()).get();
            }
            const auto& rect = texture->UpdateRect;
            rhi::TextureUploadRegion region;
            region.x = create ? 0 : rect.x;
            region.y = create ? 0 : rect.y;
            region.width = create ? texture->Width : rect.w;
            region.height = create ? texture->Height : rect.h;
            if (region.width && region.height)
            {
                const size_t rowBytes = static_cast<size_t>(region.width) * 4;
                std::vector<uint8_t> pixels(rowBytes * region.height);
                const auto* source = static_cast<const uint8_t*>(texture->GetPixels());
                for (uint32_t row = 0; row < region.height; ++row)
                    memcpy(pixels.data() + row * rowBytes,
                           source + (static_cast<size_t>(region.y + row) * texture->Width + region.x) * 4,
                           rowBytes);
                uploads.UploadTexture(pixels, binding->texture, region,
                    create ? rhi::TextureLayout::Undefined : rhi::TextureLayout::ShaderReadOnly);
            }
            if (create)
            {
                const ImTextureID id = backend.nextTexture++;
                texture->BackendUserData = binding;
                backend.textures.emplace(id, std::move(newBinding));
                texture->SetTexID(id);
            }
            texture->SetStatus(ImTextureStatus_OK);
        }
    }
}

void ImGui_ImplRenderGraph_Shutdown()
{
    auto& io = ImGui::GetIO();
    auto& backend = GetBackend();
    for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
    {
        if (texture->BackendUserData && backend.textures.contains(texture->GetTexID()))
        {
            texture->SetTexID(ImTextureID_Invalid);
            texture->BackendUserData = nullptr;
            texture->SetStatus(ImTextureStatus_Destroyed);
        }
    }
    delete &backend;
    io.BackendRendererUserData = nullptr;
    io.BackendRendererName = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
}

void ImGui_ImplRenderGraph_RenderDrawData(ImDrawData* data, RG::RenderGraph& graph,
    RG::AccessId<rhi::Texture2D> target, bool clear, const Vec4f& clearColor)
{
    if (!data || data->DisplaySize.x <= 0 || data->DisplaySize.y <= 0)
        return;
    const int width = static_cast<int>(data->DisplaySize.x * data->FramebufferScale.x);
    const int height = static_cast<int>(data->DisplaySize.y * data->FramebufferScale.y);
    if (width <= 0 || height <= 0)
        return;
    auto& backend = GetBackend();
    auto* targetResource = graph.GetVirtualResourceById(target);
    IM_ASSERT(targetResource && targetResource->desc.pixelFormat == backend.colorFormat);
    std::string tag = "ImGui." + graph.CreateUniqueId();
    std::vector<RG::AccessId<rhi::Texture2D>> sampledTextures;
    std::unordered_set<ImTextureID> usedTextures;
    for (const ImDrawList* list : data->CmdLists)
        for (const ImDrawCmd& command : list->CmdBuffer)
            if (!command.UserCallback && command.ElemCount)
                usedTextures.insert(command.GetTexID());
    for (ImTextureID id : usedTextures)
    {
        auto found = backend.textures.find(id);
        if (found == backend.textures.end())
            throw std::runtime_error("ImGui: unregistered texture ID");
        auto& texture = *found->second->texture;
        RG::TextureDesc desc{};
        desc.width = texture.GetWidth();
        desc.height = texture.GetHeight();
        desc.pixelFormat = texture.GetFormat();
        desc.usages = texture.GetUsages();
        desc.layout = rhi::TextureLayout::ShaderReadOnly;
        auto resource = graph.GetResourceArena().Import(&texture);
        sampledTextures.push_back(graph.Import<rhi::Texture2D>(tag + ".Texture." + std::to_string(id), desc,
            std::span<const RG::ResourceId<rhi::Texture2D>>(&resource, 1)));
    }
    graph.AddRenderTask<DrawTask>(tag,
        [&](RG::RenderTaskBuilder& builder, DrawTask& task) {
            task.backend = &backend;
            task.drawData = data;
            task.width = std::min(static_cast<uint32_t>(width), targetResource->desc.width);
            task.height = std::min(static_cast<uint32_t>(height), targetResource->desc.height);
            if (data->TotalVtxCount > 0 && data->TotalIdxCount > 0)
            {
                task.vertices = builder.Create<rhi::VertexBuffer>(tag + ".Vertices",
                    RG::VertexBufferDesc{static_cast<size_t>(data->TotalVtxCount) * sizeof(ImDrawVert)});
                task.indices = builder.Create<rhi::IndexBuffer>(tag + ".Indices",
                    RG::IndexBufferDesc{static_cast<size_t>(data->TotalIdxCount) * sizeof(ImDrawIdx)});
                builder.Write(task.vertices);
                builder.Read(task.vertices);
                builder.Write(task.indices);
                builder.Read(task.indices);
            }
            for (auto texture : sampledTextures)
                builder.Read(texture);
            auto view = builder.Create<rhi::TextureView>(tag + ".Target", RG::TextureViewDesc{target, {}});
            RG::RenderPassDesc pass{};
            pass.colorAttachmentCount = 1;
            pass.colorAttachment[0] = {view, clear ? rhi::AttachmentLoadOp::Clear : rhi::AttachmentLoadOp::Load,
                                      rhi::AttachmentStoreOp::Store};
            pass.clearColor[0] = clearColor;
            pass.width = targetResource->desc.width;
            pass.height = targetResource->desc.height;
            builder.SetRenderPassDesc(pass);
        }, Draw);
}
