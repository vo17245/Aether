#include <Render/RHI/Pipeline.h>

namespace Aether::rhi
{
static VkFormat VertexAttributeFormatToVkFormat(VertexAttributeFormat format)
{
    switch (format)
    {
    case VertexAttributeFormat::Float32:
        return VK_FORMAT_R32_SFLOAT;
    case VertexAttributeFormat::Vec2f:
        return VK_FORMAT_R32G32_SFLOAT;
    case VertexAttributeFormat::Vec3f:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case VertexAttributeFormat::Vec4f:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case VertexAttributeFormat::UInt32:
        return VK_FORMAT_R32_UINT;
    default:
        assert(false && "unsupported vertex attribute format");
        return VK_FORMAT_UNDEFINED;
    }
}

static std::optional<vk::GraphicsPipeline> CreateVulkanPipeline(const PipelineDesc& desc)
{
    if (!desc.vertexShader || !desc.pixelShader)
    {
        assert(false && "pipeline shaders must not be null");
        return std::nullopt;
    }

    std::vector<VkFormat> colorFormats;
    colorFormats.reserve(desc.colorAttachmentFormats.size());
    for (auto format : desc.colorAttachmentFormats)
    {
        colorFormats.push_back(PixelFormatToVkFormat(format));
    }

    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    if (desc.depthAttachmentFormat.has_value())
    {
        depthFormat = PixelFormatToVkFormat(desc.depthAttachmentFormat.value());
    }

    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;
    if (desc.stencilAttachmentFormat.has_value())
    {
        stencilFormat = PixelFormatToVkFormat(desc.stencilAttachmentFormat.value());
    }

    auto layoutOpt = vk::PipelineLayout::Create();
    if (!layoutOpt)
    {
        assert(false && "failed to create pipeline layout");
        return std::nullopt;
    }

    std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
    vertexBindingDescriptions.reserve(desc.vertexLayout.bufferViews.size());
    for (uint32_t i = 0; i < desc.vertexLayout.bufferViews.size(); ++i)
    {
        vertexBindingDescriptions.push_back(VkVertexInputBindingDescription{
            .binding = i,
            .stride = desc.vertexLayout.bufferViews[i].stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.reserve(desc.vertexLayout.attributes.size());
    for (uint32_t i = 0; i < desc.vertexLayout.attributes.size(); ++i)
    {
        const auto& attribute = desc.vertexLayout.attributes[i];
        attributeDescriptions.push_back(VkVertexInputAttributeDescription{
            .location = i,
            .binding = attribute.bufferViewIndex,
            .format = VertexAttributeFormatToVkFormat(attribute.format),
            .offset = attribute.offset,
        });
    }

    vk::GraphicsPipeline::Builder builder(*layoutOpt);
    builder.SetDynamicRenderingFormats(colorFormats, depthFormat, stencilFormat)
        .PushVertexInputLayout(vertexBindingDescriptions, attributeDescriptions)
        .AddVertexStage(desc.vertexShader->GetVk(), "main")
        .AddFragmentStage(desc.pixelShader->GetVk(), "main");

    for (size_t i = 0; i < desc.colorAttachmentFormats.size(); ++i)
    {
        builder.BeginColorAttachment();
        if (desc.enableBlend)
        {
            builder.EnableBlend();
        }
        builder.EndColorAttachment();
    }

    if (desc.enableDepthTest)
    {
        builder.EnableDepthTest();
    }

    auto pipelineOpt = builder.Build();
    if (!pipelineOpt)
    {
        assert(false && "failed to create graphics pipeline");
        return std::nullopt;
    }
    return pipelineOpt;
}
Pipeline Pipeline::Create(const PipelineDesc& desc)
{
    Pipeline pipeline;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto vkPipeline = CreateVulkanPipeline(desc);
        if (vkPipeline)
        {
            pipeline.m_Pipeline = std::move(*vkPipeline);
        }
    }
    break;
    default:
        assert(false && "unsupported render api");
        break;
    }
    return pipeline;
}
} // namespace Aether::rhi
