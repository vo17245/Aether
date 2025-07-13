#include "DeviceRenderPass.h"
namespace Aether
{
VkRenderPassCreateInfo DeviceRenderPassDescToVk(const DeviceRenderPassDesc& desc, VkRenderPassCreateInfoStorage& storage)
{
    // config depth attachment
    if (desc.depthAttachment)
    {
        VkAttachmentDescription& depthAttachment = storage.attachs[DeviceRenderPassDesc::MaxColorAttachments];
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = DeviceRenderPassDesc::MaxColorAttachments;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    // config color attachment
    for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        auto& colorAttachmentDesc = desc.colorAttachments[i];
        VkAttachmentDescription& colorAttachment = storage.attachs[0];
        colorAttachment.format = PixelFormatToVkFormat(colorAttachmentDesc.format);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = DeviceAttachmentLoadOpToVkLoadOp(colorAttachmentDesc.load);
        colorAttachment.storeOp = DeviceAttachmentStoreOpToVkStoreOp(colorAttachmentDesc.store);
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        auto& attachmentRef = storage.attachRefs[i];
        attachmentRef.attachment = i;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription& subpass = storage.subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = desc.colorAttachmentCount;
    subpass.pColorAttachments = storage.attachRefs;
    subpass.pDepthStencilAttachment = storage.attachRefs + DeviceRenderPassDesc::MaxColorAttachments;

    VkSubpassDependency& dependency = storage.dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo& renderPassInfo = storage.renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = storage.attachs;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
}
} // namespace Aether