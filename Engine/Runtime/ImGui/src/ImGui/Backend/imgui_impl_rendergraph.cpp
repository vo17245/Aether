#include "imgui_impl_rendergraph.h"
#include <Render/RenderGraph/RenderGraph.h>
#include <Render/Upload/PendingUploadList.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace Aether;
namespace RG = Aether::RenderGraph;

namespace
{
struct SurfaceTokenHash
{
    std::size_t operator()(DisplaySurfaceToken token) const noexcept
    {
        const auto first = std::hash<std::uint64_t>{}(token.id);
        const auto second = std::hash<std::uint32_t>{}(token.generation);
        return first ^ (second + 0x9e3779b9u + (first << 6) + (first >> 2));
    }
};

struct TextureBinding
{
    rhi::Texture2D ownedTexture;
    rhi::TextureView ownedView;
    rhi::Texture2D* texture = nullptr;
    rhi::TextureView* view = nullptr;
    std::optional<vk::DescriptorPool> pool;
    std::optional<vk::DescriptorSet> set;
};

struct PacketSurfaceBinding
{
    std::shared_ptr<TextureBinding> binding;
    std::shared_ptr<const void> lease;
};
using PacketSurfaceBindings =
    std::unordered_map<DisplaySurfaceToken, PacketSurfaceBinding, SurfaceTokenHash>;

} // namespace

struct ImGui_ImplRenderGraph_Backend
{
    PixelFormat colorFormat;
    rhi::Sampler sampler;
    std::optional<vk::DescriptorSetLayout> descriptorLayout;
    std::optional<vk::PipelineLayout> layout;
    rhi::Pipeline pipeline;
    std::unordered_map<ImGuiCompat::TextureId, std::unique_ptr<TextureBinding>> textures;
    struct RetiredTexture
    {
        std::unique_ptr<TextureBinding> binding;
        std::array<bool, Render::Config::MaxFramesInFlight> pendingSlots{};
    };
    std::vector<RetiredTexture> retiredTextures;
    ImGuiCompat::TextureId nextTexture = 1;
    std::unordered_map<ImGuiCompat::RenderCallbackId, ImGui_ImplRenderGraph_Callback> callbacks;
    ImGui_ImplRenderGraph_DisplaySurfaceResolver displaySurfaceResolver;
    ImGuiCompat::ImGuiRenderPacketExtractor compatibilityExtractor;
};

namespace
{
using Backend = ImGui_ImplRenderGraph_Backend;

void CreateBinding(Backend& backend, TextureBinding& binding, rhi::Sampler& sampler);

Backend& GetBackend()
{
    auto* backend = static_cast<Backend*>(ImGui::GetIO().BackendRendererUserData);
    IM_ASSERT(backend && "ImGui RenderGraph backend is not initialized");
    return *backend;
}

void RetireBinding(Backend& backend, std::unique_ptr<TextureBinding> binding)
{
    if (!binding)
        return;
    Backend::RetiredTexture retired{.binding = std::move(binding)};
    retired.pendingSlots.fill(true);
    backend.retiredTextures.push_back(std::move(retired));
}

std::unique_ptr<TextureBinding> CreateOwnedBinding(Backend& backend, std::uint32_t width,
                                                   std::uint32_t height)
{
    auto binding = std::make_unique<TextureBinding>();
    rhi::TextureDesc desc{};
    desc.width = width;
    desc.height = height;
    // Alpha8 data is expanded to white RGBA with the source byte in alpha so
    // user RGBA textures and atlas textures share the same shader.
    desc.pixelFormat = PixelFormat::RGBA8888;
    desc.usages = PackFlags(rhi::TextureUsage::Sample, rhi::TextureUsage::TransferDst);
    desc.layout = rhi::TextureLayout::Undefined;
    binding->ownedTexture = rhi::Texture2D::Create(desc);
    if (!binding->ownedTexture)
        throw std::runtime_error("ImGui: failed to create owned texture");
    binding->ownedView = binding->ownedTexture.CreateImageView({});
    binding->texture = &binding->ownedTexture;
    binding->view = &binding->ownedView;
    CreateBinding(backend, *binding, backend.sampler);
    return binding;
}

std::vector<std::uint8_t> ExpandPixels(ImTextureFormat format, std::span<const std::byte> pixels)
{
    if (format == ImTextureFormat_RGBA32)
    {
        const auto* first = reinterpret_cast<const std::uint8_t*>(pixels.data());
        return {first, first + pixels.size()};
    }
    std::vector<std::uint8_t> expanded(pixels.size() * 4);
    for (std::size_t index = 0; index < pixels.size(); ++index)
    {
        expanded[index * 4 + 0] = 255;
        expanded[index * 4 + 1] = 255;
        expanded[index * 4 + 2] = 255;
        expanded[index * 4 + 3] = std::to_integer<std::uint8_t>(pixels[index]);
    }
    return expanded;
}

void QueueRegionUpload(const ImGuiCompat::ImGuiTextureOperation& operation,
                       const ImGuiCompat::ImGuiTextureRegion& source,
                       TextureBinding& binding, PendingUploadList& uploads,
                       rhi::TextureLayout oldLayout)
{
    const std::size_t sourcePixelSize = operation.format == ImTextureFormat_RGBA32 ? 4 : 1;
    if (source.rowBytes != static_cast<std::size_t>(source.width) * sourcePixelSize ||
        source.pixels.size() != static_cast<std::size_t>(source.rowBytes) * source.height)
        throw std::invalid_argument("ImGui texture operation has invalid row pitch or pixel data");
    auto pixels = ExpandPixels(operation.format, source.pixels);
    uploads.UploadTexture(pixels, binding.texture,
                          {.x = source.x, .y = source.y, .width = source.width, .height = source.height},
                          oldLayout);
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

struct PacketDrawTask
{
    Backend* backend = nullptr;
    std::shared_ptr<const ImGuiCompat::ImGuiRenderPacket> packet;
    std::shared_ptr<const PacketSurfaceBindings> displaySurfaces;
    RG::AccessId<rhi::VertexBuffer> vertices;
    RG::AccessId<rhi::IndexBuffer> indices;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

void DrawPacket(rhi::CommandList& commands, RG::ResourceAccessor& resources, PacketDrawTask& task)
{
    const auto& data = *task.packet;
    if (data.vertices.empty() || data.indices.empty())
        return;
    auto& backend = *task.backend;
    auto& vertices = resources.GetResource(task.vertices)->GetVk();
    auto& indices = resources.GetResource(task.indices)->GetVk();
    vertices.SetData(0, {reinterpret_cast<const std::uint8_t*>(data.vertices.data()),
                         data.vertices.size() * sizeof(data.vertices[0])});
    indices.SetData(0, {reinterpret_cast<const std::uint8_t*>(data.indices.data()),
                        data.indices.size() * sizeof(data.indices[0])});

    VkCommandBuffer commandBuffer = commands.GetVk().GetHandle();
    auto setup = [&]() {
        commands.BindPipeline(backend.pipeline);
        commands.SetViewport(0, 0, static_cast<float>(task.width), static_cast<float>(task.height));
        commands.SetScissor(0, 0, static_cast<float>(task.width), static_cast<float>(task.height));
        VkBuffer vertexBuffer = vertices.GetHandle();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, indices.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
        float transform[4] = {
            2.0f / data.displaySize[0],
            2.0f / data.displaySize[1],
            -1.0f - data.displayPos[0] * (2.0f / data.displaySize[0]),
            -1.0f - data.displayPos[1] * (2.0f / data.displaySize[1]),
        };
        vkCmdPushConstants(commandBuffer, backend.layout->GetHandle(), VK_SHADER_STAGE_VERTEX_BIT,
                           0, sizeof(transform), transform);
    };
    setup();

    for (const auto& command : data.commands)
    {
        if (command.type == ImGuiCompat::ImGuiPacketCommandType::ResetRenderState)
        {
            setup();
            continue;
        }
        if (command.type == ImGuiCompat::ImGuiPacketCommandType::Callback)
        {
            auto callback = backend.callbacks.find(command.callback);
            if (callback == backend.callbacks.end())
                throw std::runtime_error("ImGui packet references an unregistered callback");
            callback->second(commands, command.callbackPayload);
            continue;
        }
        if (command.elementCount == 0)
            continue;

        float x1 = (command.clipRect[0] - data.displayPos[0]) * data.framebufferScale[0];
        float y1 = (command.clipRect[1] - data.displayPos[1]) * data.framebufferScale[1];
        float x2 = (command.clipRect[2] - data.displayPos[0]) * data.framebufferScale[0];
        float y2 = (command.clipRect[3] - data.displayPos[1]) * data.framebufferScale[1];
        x1 = std::clamp(x1, 0.0f, static_cast<float>(task.width));
        y1 = std::clamp(y1, 0.0f, static_cast<float>(task.height));
        x2 = std::clamp(x2, 0.0f, static_cast<float>(task.width));
        y2 = std::clamp(y2, 0.0f, static_cast<float>(task.height));
        if (x2 <= x1 || y2 <= y1)
            continue;
        const TextureBinding* textureBinding = nullptr;
        if (command.displaySurface)
        {
            auto binding = task.displaySurfaces->find(*command.displaySurface);
            if (binding == task.displaySurfaces->end())
                throw std::runtime_error("ImGui packet references an unresolved display surface");
            textureBinding = binding->second.binding.get();
        }
        else
        {
            auto binding = backend.textures.find(command.texture);
            if (binding == backend.textures.end())
                throw std::runtime_error("ImGui packet references an unregistered texture");
            textureBinding = binding->second.get();
        }
        commands.SetScissor(x1, y1, x2 - x1, y2 - y1);
        VkDescriptorSet descriptor = textureBinding->set->GetHandle();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                backend.layout->GetHandle(), 0, 1, &descriptor, 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, command.elementCount, 1, command.indexOffset,
                         static_cast<std::int32_t>(command.vertexOffset), 0);
    }
    commands.SetScissor(0, 0, static_cast<float>(task.width), static_cast<float>(task.height));
}
} // namespace

bool ImGui_ImplRenderGraph_Init(const ImGui_ImplRenderGraph_InitInfo& info)
{
    auto& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr);
    auto* backend = ImGui_ImplRenderGraph_CreateBackend(info);
    if (!backend)
        return false;
    io.BackendRendererUserData = backend;
    io.BackendRendererName = "imgui_impl_rendergraph";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
    return true;
}

ImGui_ImplRenderGraph_Backend* ImGui_ImplRenderGraph_CreateBackend(
    const ImGui_ImplRenderGraph_InitInfo& info)
{
    auto backend = std::make_unique<Backend>();
    backend->colorFormat = info.ColorFormat;
    auto sampler = vk::Sampler::Builder().SetDefaultCreateInfo()
        .SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE).SetAnisotropyEnable(false).Build();
    if (!sampler)
        return nullptr;
    backend->sampler = rhi::Sampler(std::move(*sampler));
    backend->descriptorLayout = vk::DescriptorSetLayout::Builder()
        .BeginSamplerBinding().UseFragmentStage().EndSamplerBinding().Build();
    if (!backend->descriptorLayout)
        return nullptr;
    backend->layout = vk::PipelineLayout::Builder().AddDescriptorSetLayout(*backend->descriptorLayout)
        .AddPushConstantRange(4 * sizeof(float), static_cast<vk::ShaderStageFlags>(VK_SHADER_STAGE_VERTEX_BIT)).Build();
    if (!backend->layout)
        return nullptr;
    std::string preamble;
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
    preamble = "#define IMGUI_COLOR_BGRA\n";
#endif
    auto vertex = rhi::VertexShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, VertexCode, preamble));
    auto pixel = rhi::PixelShader::Create(ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, PixelCode));
    if (!vertex || !pixel)
        return nullptr;
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
        return nullptr;
    backend->pipeline = rhi::Pipeline(std::move(*pipeline));
    return backend.release();
}

void ImGui_ImplRenderGraph_DestroyBackend(ImGui_ImplRenderGraph_Backend* backend)
{
    delete backend;
}

void ImGui_ImplRenderGraph_NewFrame()
{
    (void)GetBackend();
}

ImGui_ImplRenderGraph_Backend* ImGui_ImplRenderGraph_GetBackend()
{
    return &GetBackend();
}

ImTextureID ImGui_ImplRenderGraph_AddTexture(rhi::Texture2D& texture, rhi::TextureView& view, rhi::Sampler& sampler)
{
    auto& backend = GetBackend();
    auto binding = std::make_unique<TextureBinding>();
    binding->texture = &texture;
    binding->view = &view;
    CreateBinding(backend, *binding, sampler);
    const ImTextureID id = static_cast<ImTextureID>(backend.nextTexture++);
    backend.textures.emplace(id, std::move(binding));
    return id;
}

void ImGui_ImplRenderGraph_RemoveTexture(ImTextureID texture)
{
    auto& backend = GetBackend();
    auto found = backend.textures.find(static_cast<ImGuiCompat::TextureId>(texture));
    if (found == backend.textures.end())
        return;
    RetireBinding(backend, std::move(found->second));
    backend.textures.erase(found);
}

void ImGui_ImplRenderGraph_UpdateTextures(ImDrawData* data, PendingUploadList& uploads)
{
    auto& backend = GetBackend();
    if (!data)
        return;
    auto extraction = backend.compatibilityExtractor.Extract(*data);
    if (!extraction)
        throw std::runtime_error(extraction.error().message);
    ImGui_ImplRenderGraph_ApplyTextureOperations(backend, extraction->textureOperations, uploads);
    if (!backend.compatibilityExtractor.CommitAccepted(*extraction))
        throw std::runtime_error("ImGui texture extraction was acknowledged twice");
}

void ImGui_ImplRenderGraph_ApplyTextureOperations(
    ImGui_ImplRenderGraph_Backend& backend,
    std::span<const ImGuiCompat::ImGuiTextureOperation> operations,
    PendingUploadList& uploads)
{
    for (const auto& operation : operations)
    {
        if (operation.texture == 0)
            throw std::invalid_argument("ImGui texture operation has invalid identity");

        auto found = backend.textures.find(operation.texture);
        if (operation.type == ImGuiCompat::ImGuiTextureOperationType::Destroy)
        {
            if (found != backend.textures.end())
            {
                RetireBinding(backend, std::move(found->second));
                backend.textures.erase(found);
            }
            continue;
        }

        if (operation.width == 0 || operation.height == 0)
            throw std::invalid_argument("ImGui texture operation has invalid dimensions");

        if (operation.format != ImTextureFormat_RGBA32 && operation.format != ImTextureFormat_Alpha8)
            throw std::invalid_argument("ImGui texture operation has unsupported format");

        auto replacement = CreateOwnedBinding(backend, operation.width, operation.height);
        if (operation.type == ImGuiCompat::ImGuiTextureOperationType::Create)
        {
            if (found != backend.textures.end())
                throw std::logic_error("ImGui create operation reused a live texture ID");
            if (operation.regions.size() != 1 || operation.regions[0].x != 0 ||
                operation.regions[0].y != 0 || operation.regions[0].width != operation.width ||
                operation.regions[0].height != operation.height)
                throw std::invalid_argument("ImGui create operation must own one full image");
            QueueRegionUpload(operation, operation.regions[0], *replacement, uploads,
                              rhi::TextureLayout::Undefined);
            backend.textures.emplace(operation.texture, std::move(replacement));
            continue;
        }

        if (found == backend.textures.end())
            throw std::logic_error("ImGui update operation references an unknown texture ID");
        const std::size_t sourcePixelSize = operation.format == ImTextureFormat_RGBA32 ? 4 : 1;
        if (operation.fullRowBytes != operation.width * sourcePixelSize ||
            operation.fullPixels.size() != static_cast<std::size_t>(operation.fullRowBytes) * operation.height)
            throw std::invalid_argument("ImGui update operation does not own a full replacement image");
        ImGuiCompat::ImGuiTextureRegion full{
            .x = 0,
            .y = 0,
            .width = operation.width,
            .height = operation.height,
            .rowBytes = operation.fullRowBytes,
            .pixels = operation.fullPixels,
        };
        QueueRegionUpload(operation, full, *replacement, uploads, rhi::TextureLayout::Undefined);
        auto retired = std::move(found->second);
        found->second = std::move(replacement);
        RetireBinding(backend, std::move(retired));
    }
}

void ImGui_ImplRenderGraph_OnFrameSlotCompleted(ImGui_ImplRenderGraph_Backend& backend,
                                                 std::uint32_t frameSlot)
{
    if (frameSlot >= Render::Config::MaxFramesInFlight)
        throw std::out_of_range("ImGui completed frame slot is out of range");
    for (auto& texture : backend.retiredTextures)
        texture.pendingSlots[frameSlot] = false;
    std::erase_if(backend.retiredTextures, [](const Backend::RetiredTexture& texture) {
        return std::none_of(texture.pendingSlots.begin(), texture.pendingSlots.end(),
                            [](bool pending) { return pending; });
    });
}

void ImGui_ImplRenderGraph_RegisterCallback(ImGui_ImplRenderGraph_Backend& backend,
                                            ImGuiCompat::RenderCallbackId id,
                                            ImGui_ImplRenderGraph_Callback callback)
{
    if (id == 0 || !callback)
        throw std::invalid_argument("ImGui render callback requires a non-zero ID and callable");
    backend.callbacks[id] = std::move(callback);
}

void ImGui_ImplRenderGraph_UnregisterCallback(ImGui_ImplRenderGraph_Backend& backend,
                                              ImGuiCompat::RenderCallbackId id)
{
    backend.callbacks.erase(id);
}

void ImGui_ImplRenderGraph_SetDisplaySurfaceResolver(
    ImGui_ImplRenderGraph_Backend& backend, ImGui_ImplRenderGraph_DisplaySurfaceResolver resolver)
{
    backend.displaySurfaceResolver = std::move(resolver);
}

void ImGui_ImplRenderGraph_Shutdown()
{
    auto& io = ImGui::GetIO();
    auto& backend = GetBackend();
    for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
    {
        if (texture->GetTexID() != ImTextureID_Invalid &&
            backend.textures.contains(static_cast<ImGuiCompat::TextureId>(texture->GetTexID())))
        {
            texture->SetTexID(ImTextureID_Invalid);
            texture->BackendUserData = nullptr;
            texture->SetStatus(ImTextureStatus_Destroyed);
        }
    }
    ImGui_ImplRenderGraph_DestroyBackend(&backend);
    io.BackendRendererUserData = nullptr;
    io.BackendRendererName = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
}

void ImGui_ImplRenderGraph_RenderDrawData(ImDrawData* data, RG::RenderGraph& graph,
    RG::AccessId<rhi::Texture2D> target, bool clear, const Vec4f& clearColor)
{
    if (!data)
        return;
    auto& backend = GetBackend();
    auto extraction = backend.compatibilityExtractor.Extract(*data);
    if (!extraction)
        throw std::runtime_error(extraction.error().message);
    if (!extraction->textureOperations.empty())
        throw std::logic_error("ImGui texture operations must be applied before rendering draw data");
    ImGui_ImplRenderGraph_RenderPacket(backend,
        std::make_shared<const ImGuiCompat::ImGuiRenderPacket>(std::move(extraction->packet)),
        graph, target, clear, clearColor);
}

void ImGui_ImplRenderGraph_RenderPacket(
    ImGui_ImplRenderGraph_Backend& backend,
    std::shared_ptr<const ImGuiCompat::ImGuiRenderPacket> packet,
    RG::RenderGraph& graph, RG::AccessId<rhi::Texture2D> target,
    bool clear, const Vec4f& clearColor, std::uint32_t frameSlot)
{
    if (!packet || packet->displaySize[0] <= 0.0f || packet->displaySize[1] <= 0.0f)
        return;
    const auto width = static_cast<std::uint32_t>(
        packet->displaySize[0] * packet->framebufferScale[0]);
    const auto height = static_cast<std::uint32_t>(
        packet->displaySize[1] * packet->framebufferScale[1]);
    if (width == 0 || height == 0)
        return;
    auto* targetResource = graph.GetVirtualResourceById(target);
    if (!targetResource || targetResource->desc.pixelFormat != backend.colorFormat)
        throw std::invalid_argument("ImGui packet target has an incompatible format");
    std::string tag = "ImGui." + graph.CreateUniqueId();
    std::vector<RG::AccessId<rhi::Texture2D>> sampledTextures;
    std::unordered_set<ImGuiCompat::TextureId> usedTextures;
    std::unordered_set<DisplaySurfaceToken, SurfaceTokenHash> usedDisplaySurfaces;
    for (const auto& command : packet->commands)
        if (command.type == ImGuiCompat::ImGuiPacketCommandType::Draw && command.elementCount)
        {
            if (command.displaySurface)
                usedDisplaySurfaces.insert(*command.displaySurface);
            else
                usedTextures.insert(command.texture);
        }
    for (const auto id : usedTextures)
    {
        auto found = backend.textures.find(id);
        if (found == backend.textures.end())
            throw std::runtime_error("ImGui packet uses an unregistered texture ID");
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
    auto surfaceBindings = std::make_shared<PacketSurfaceBindings>();
    for (const auto token : usedDisplaySurfaces)
    {
        if (!backend.displaySurfaceResolver)
            throw std::runtime_error("ImGui packet uses a display surface without a resolver");
        const auto command = std::ranges::find_if(packet->commands, [token](const auto& item) {
            return item.type == ImGuiCompat::ImGuiPacketCommandType::Draw && item.displaySurface == token;
        });
        if (command == packet->commands.end())
            throw std::logic_error("ImGui display surface lost its packet lifetime pin");
        auto resolved = backend.displaySurfaceResolver(token, command->displaySurfacePin, frameSlot);
        if (!resolved || !resolved->texture || !resolved->view || !resolved->sampler || !resolved->lease)
            throw std::runtime_error("ImGui display surface is unavailable or has no lifetime lease");

        auto binding = std::make_shared<TextureBinding>();
        binding->texture = resolved->texture;
        binding->view = resolved->view;
        CreateBinding(backend, *binding, *resolved->sampler);

        RG::TextureDesc desc{};
        desc.width = resolved->texture->GetWidth();
        desc.height = resolved->texture->GetHeight();
        desc.pixelFormat = resolved->texture->GetFormat();
        desc.usages = resolved->texture->GetUsages();
        desc.layout = rhi::TextureLayout::ShaderReadOnly;
        if (!resolved->resourceId.handle.IsValid())
            throw std::runtime_error("ImGui display surface has no stable ResourceArena registration");
        const auto surfaceAccess = graph.Import<rhi::Texture2D>(
            tag + ".Surface." + std::to_string(token.id) + "." + std::to_string(token.generation),
            desc, std::span<const RG::ResourceId<rhi::Texture2D>>(&resolved->resourceId, 1));
        sampledTextures.push_back(surfaceAccess);
        surfaceBindings->emplace(token, PacketSurfaceBinding{std::move(binding), std::move(resolved->lease)});
    }
    graph.AddRenderTask<PacketDrawTask>(tag,
        [&](RG::RenderTaskBuilder& builder, PacketDrawTask& task) {
            task.backend = &backend;
            task.packet = packet;
            task.displaySurfaces = surfaceBindings;
            task.width = std::min(width, targetResource->desc.width);
            task.height = std::min(height, targetResource->desc.height);
            if (!packet->vertices.empty() && !packet->indices.empty())
            {
                task.vertices = builder.Create<rhi::VertexBuffer>(tag + ".Vertices",
                    RG::VertexBufferDesc{packet->vertices.size() * sizeof(packet->vertices[0])});
                task.indices = builder.Create<rhi::IndexBuffer>(tag + ".Indices",
                    RG::IndexBufferDesc{packet->indices.size() * sizeof(packet->indices[0])});
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
        }, DrawPacket);
}
