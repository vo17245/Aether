#include <Render/RHI/Pipeline.h>
namespace Aether::rhi
{
static vk::GraphicsPipeline CreateVulkanPipeline(const PipelineDesc& desc)
{
    std::vector<VkFormat> colorFormats;
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
    VkPipelineRenderingCreateInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = static_cast<uint32_t>(colorFormats.size()),
        .pColorAttachmentFormats = colorFormats.data(),
        .depthAttachmentFormat = depthFormat,
        .stencilAttachmentFormat = stencilFormat,
    };
}
Pipeline Pipeline::Create(const PipelineDesc& desc)
{
    Pipeline pipeline;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        pipeline.m_Pipeline = CreateVulkanPipeline(desc);
    }
    break;
    default:
        assert(false && "unsupported render api");
        break;
    }
    return pipeline;
}
} // namespace Aether::rhi