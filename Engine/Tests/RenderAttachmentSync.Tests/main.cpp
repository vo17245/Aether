#include <Entry/Application.h>
#include <Render/Feature/RenderFeature.h>
#include <Render/PixelFormat.h>
#include <Render/RHI/Backend/Vulkan/Pipeline.h>
#include <Render/RHI/Backend/Vulkan/PipelineLayout.h>
#include <Render/RenderGraph/RenderTask.h>
#include <Render/RenderGraph/Resource/Texture2D.h>
#include <Render/RenderGraph/Resource/TextureView.h>
#include <Window/Layer.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
using namespace Aether;
namespace RG = Aether::RenderGraph;
namespace rhi = Aether::rhi;
namespace vk = Aether::vk;

void Require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

struct PushData
{
    float color[4];
    float depth;
};
static_assert(sizeof(PushData) == 20);

constexpr const char* VertexShader = R"GLSL(#version 450
void main()
{
    vec2 points[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    gl_Position = vec4(points[gl_VertexIndex], 0.0, 1.0);
}
)GLSL";
constexpr const char* FragmentShader = R"GLSL(#version 450
layout(push_constant) uniform Params { vec4 color; float depth; } params;
layout(location = 0) out vec4 outColor;
void main()
{
    outColor = params.color;
    gl_FragDepth = params.depth;
}
)GLSL";

struct ScopeTask
{
    RG::AccessId<rhi::Texture2D> depth;
    RG::AccessId<rhi::TextureView> colorView;
    RG::AccessId<rhi::TextureView> depthView;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    bool first = false;
};

struct EmptyFrameData final : Render::RenderFeatureData {};

class AttachmentSyncFeature final : public Render::RenderFeature
{
public:
    void OnRenderDetach(Render::RenderFrameContext&) override
    {
        blended = {};
        initial = {};
        layout.reset();
        format = VK_FORMAT_UNDEFINED;
    }

    void BuildRenderGraph(Render::RenderGraphBuildContext& context) override
    {
        const auto* target = context.graph.GetVirtualResourceById(context.output);
        Require(target != nullptr, "missing final color image");
        Require(context.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                "unexpected output color space");
        const auto attachmentFormat = PixelFormatToVkFormat(target->desc.pixelFormat);
        Require(attachmentFormat == VK_FORMAT_R8G8B8A8_UNORM || attachmentFormat == VK_FORMAT_B8G8R8A8_UNORM,
                "output attachment is not an 8-bit UNORM image");
        if (format != attachmentFormat) BuildPipelines(attachmentFormat);

        const auto tag = "AttachmentSync." + context.graph.CreateUniqueId();
        auto firstPass = context.graph.AddRenderTask<ScopeTask>(tag + ".write", [&](RG::RenderTaskBuilder& builder,
                                                                                     ScopeTask& task) {
            task.first = true;
            task.width = context.width;
            task.height = context.height;
            task.depth = builder.Create<rhi::Texture2D>(tag + ".Depth", RG::TextureDesc{
                .usages = PackFlags(rhi::TextureUsage::DepthAttachment),
                .pixelFormat = PixelFormat::R_FLOAT32_DEPTH,
                .width = context.width,
                .height = context.height,
                .layout = rhi::TextureLayout::DepthStencilAttachment});
            task.colorView = builder.Create<rhi::TextureView>(tag + ".ColorView.Write",
                RG::TextureViewDesc{context.output, {}});
            task.depthView = builder.Create<rhi::TextureView>(tag + ".DepthView.Write",
                RG::TextureViewDesc{task.depth, {}});
            RG::RenderPassDesc pass{};
            pass.colorAttachmentCount = 1;
            pass.colorAttachment[0] = {task.colorView, rhi::AttachmentLoadOp::Clear, rhi::AttachmentStoreOp::Store};
            pass.clearColor[0] = {0.0f, 1.0f, 0.0f, 1.0f};
            pass.depthAttachment = RG::Attachment{task.depthView, rhi::AttachmentLoadOp::Clear,
                                                   rhi::AttachmentStoreOp::Store};
            pass.clearDepth = 1.0f;
            pass.width = context.width;
            pass.height = context.height;
            builder.SetRenderPassDesc(pass);
        }, [this](rhi::CommandList& commands, RG::ResourceAccessor&, ScopeTask& task) {
            Draw(commands, task, initial, PushData{{0.0f, 1.0f, 0.0f, 1.0f}, 0.5f});
        });

        context.graph.AddRenderTask<ScopeTask>(tag + ".load-and-test", [&](RG::RenderTaskBuilder& builder,
                                                                              ScopeTask& task) {
            task.first = false;
            task.depth = firstPass.depth;
            task.width = context.width;
            task.height = context.height;
            task.colorView = builder.Create<rhi::TextureView>(tag + ".ColorView.Read",
                RG::TextureViewDesc{context.output, {}});
            task.depthView = builder.Create<rhi::TextureView>(tag + ".DepthView.Read",
                RG::TextureViewDesc{task.depth, {}});
            RG::RenderPassDesc pass{};
            pass.colorAttachmentCount = 1;
            pass.colorAttachment[0] = {task.colorView, rhi::AttachmentLoadOp::Load, rhi::AttachmentStoreOp::Store};
            pass.depthAttachment = RG::Attachment{task.depthView, rhi::AttachmentLoadOp::Load,
                                                   rhi::AttachmentStoreOp::Store};
            pass.width = context.width;
            pass.height = context.height;
            builder.SetRenderPassDesc(pass);
        }, [this](rhi::CommandList& commands, RG::ResourceAccessor&, ScopeTask& task) {
            Draw(commands, task, blended, PushData{{1.0f, 0.0f, 0.0f, 1.0f}, 0.75f});
            Draw(commands, task, blended, PushData{{0.0f, 0.0f, 1.0f, 0.5f}, 0.25f});
        });
    }

private:
    void BuildPipelines(VkFormat targetFormat)
    {
        blended = {};
        initial = {};
        layout.reset();
        format = VK_FORMAT_UNDEFINED;
        auto pipelineLayout = vk::PipelineLayout::Builder()
            .AddPushConstantRange(sizeof(PushData), static_cast<vk::ShaderStageFlags>(VK_SHADER_STAGE_FRAGMENT_BIT))
            .Build();
        if (!pipelineLayout) throw std::runtime_error("attachment sync pipeline layout creation failed");
        layout = std::move(*pipelineLayout);
        auto vertex = rhi::VertexShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, VertexShader));
        auto fragment = rhi::PixelShader::Create(ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, FragmentShader));
        if (!vertex || !fragment) throw std::runtime_error("attachment sync shaders failed");
        auto firstPipeline = vk::GraphicsPipeline::Builder(*layout)
            .SetDynamicRenderingFormats({&targetFormat, 1}, VK_FORMAT_D32_SFLOAT)
            .SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .EnableDepthTest().SetDepthTestCompareFunc(VK_COMPARE_OP_LESS_OR_EQUAL).SetDepthWriteEnable(true)
            .AddVertexStage(vertex->GetVk(), "main").AddFragmentStage(fragment->GetVk(), "main")
            .BeginColorAttachment().EndColorAttachment().Build();
        auto blendedPipeline = vk::GraphicsPipeline::Builder(*layout)
            .SetDynamicRenderingFormats({&targetFormat, 1}, VK_FORMAT_D32_SFLOAT)
            .SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .EnableDepthTest().SetDepthTestCompareFunc(VK_COMPARE_OP_LESS_OR_EQUAL).SetDepthWriteEnable(false)
            .AddVertexStage(vertex->GetVk(), "main").AddFragmentStage(fragment->GetVk(), "main")
            .BeginColorAttachment().EnableBlend(true).EndColorAttachment().Build();
        if (!firstPipeline || !blendedPipeline) throw std::runtime_error("attachment sync pipelines failed");
        initial = rhi::Pipeline(std::move(*firstPipeline));
        blended = rhi::Pipeline(std::move(*blendedPipeline));
        format = targetFormat;
    }

    void Draw(rhi::CommandList& commands, ScopeTask& task, rhi::Pipeline& pipeline, const PushData& data)
    {
        commands.SetViewport(0.0f, 0.0f, static_cast<float>(task.width), static_cast<float>(task.height));
        commands.SetScissor(0.0f, 0.0f, static_cast<float>(task.width), static_cast<float>(task.height));
        commands.BindPipeline(pipeline);
        const auto command = commands.GetVk().GetHandle();
        vkCmdPushConstants(command, layout->GetHandle(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(data), &data);
        vkCmdDraw(command, 3, 1, 0, 0);
    }

    std::optional<vk::PipelineLayout> layout;
    rhi::Pipeline initial;
    rhi::Pipeline blended;
    VkFormat format = VK_FORMAT_UNDEFINED;
};

class SyncLayer final : public Layer
{
public:
    SyncLayer() : feature(std::make_shared<AttachmentSyncFeature>()) {}
    void CollectRenderFeatures(std::vector<std::shared_ptr<Render::RenderFeature>>& features) override
    { features.push_back(feature); }
    void ExtractRenderData(Render::RenderFeatureFrame& frame) override
    { frame.Emplace<EmptyFrameData>(feature); }
    std::shared_ptr<AttachmentSyncFeature> feature;
};

void CheckPixels(rhi::Texture2D& texture)
{
    const auto width = texture.GetWidth(), height = texture.GetHeight();
    const std::size_t bytes = static_cast<std::size_t>(width) * height * 4;
    auto readback = vk::Buffer::Create(bytes, vk::Buffer::Usage::TransferDst, vk::Buffer::Property::HostVisible);
    auto commands = vk::GraphicsCommandBuffer::Create(vk::GRC::GetGraphicsCommandPool());
    auto fence = vk::Fence::Create();
    Require(readback && commands && fence, "attachment sync readback allocation failed");
    commands->BeginSingleTime();
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.GetVk().GetHandle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(commands->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageExtent = {width, height, 1};
    vkCmdCopyImageToBuffer(commands->GetHandle(), barrier.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           readback->GetHandle(), 1, &copy);
    std::swap(barrier.oldLayout, barrier.newLayout);
    std::swap(barrier.srcAccessMask, barrier.dstAccessMask);
    vkCmdPipelineBarrier(commands->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    VkMemoryBarrier hostBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    hostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    hostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(commands->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                         0, 1, &hostBarrier, 0, nullptr, 0, nullptr);
    commands->End();
    commands->BeginSubmit().Fence(*fence).EndSubmit();
    fence->Wait();
    auto* pixels = readback->BeginMap<std::uint8_t>();
    readback->Invalidate();
    const auto* center = pixels + (static_cast<std::size_t>(height / 2) * width + width / 2) * 4;
    const int expected[] = {0, 127, 128, 255};
    for (int channel = 0; channel < 4; ++channel)
        if (std::abs(static_cast<int>(center[channel]) - expected[channel]) > 3)
        {
            std::fprintf(stderr, "Center pixel was %u,%u,%u,%u\n", center[0], center[1], center[2], center[3]);
            throw std::runtime_error("separate attachment scopes failed color load/blend/depth assertions");
        }
    readback->EndMap();
}

class SyncApplication final : public Application
{
public:
    WindowCreateParam MainWindowCreateParam() override
    {
        WindowCreateParam param;
        param.title = "Render attachment scope synchronization smoke";
        param.width = 320;
        param.height = 240;
        param.imGuiEnableClear = false;
        param.enableSynchronizationValidation = true;
        return param;
    }
    void OnInit(Window& value) override
    {
        window = &value;
        window->PushLayer(&layer);
    }
    void OnFrameBegin() override
    {
        if (++frames == 12) Quit();
    }
    void OnShutdown() override
    {
        if (window)
        {
            CheckPixels(window->GetFinalTexture(0));
            CheckPixels(window->GetFinalTexture(1));
        }
        if (window) window->PopLayer(&layer);
        std::puts("RenderAttachmentSync GPU and synchronization-validation smoke passed");
    }
private:
    Window* window = nullptr;
    SyncLayer layer;
    unsigned frames = 0;
};
} // namespace

DEFINE_APPLICATION(SyncApplication)
