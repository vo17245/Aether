#include "DeviceRenderPass.h"
namespace Aether
{
void DeviceRenderPassDescToVk(const DeviceRenderPassDesc& desc, VkRenderPassCreateInfoStorage& storage)
{
    size_t attachments = 0;
    

    // config color attachment
    for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        auto& colorAttachmentDesc = desc.colorAttachments[i];
        VkAttachmentDescription& colorAttachment = storage.attachs[i];
        colorAttachment.format = PixelFormatToVkFormat(colorAttachmentDesc.format);
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = DeviceAttachmentLoadOpToVkLoadOp(colorAttachmentDesc.loadOp);
        colorAttachment.storeOp = DeviceAttachmentStoreOpToVkStoreOp(colorAttachmentDesc.storeOp);
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if(colorAttachmentDesc.loadOp!=DeviceAttachmentLoadOp::DontCare)
        {
            colorAttachment.initialLayout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else 
        {
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        auto& attachmentRef = storage.colorAttachRefs[i];
        attachmentRef.attachment = i;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ++attachments;
    }
    // config depth attachment
    if (desc.depthAttachment)
    {
        VkAttachmentDescription& depthAttachment = storage.attachs[attachments];
        auto& depthAttachmentDesc = desc.depthAttachment.value();
        depthAttachment.format = PixelFormatToVkFormat(depthAttachmentDesc.format);
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = DeviceAttachmentLoadOpToVkLoadOp(depthAttachmentDesc.loadOp);
        depthAttachment.storeOp = DeviceAttachmentStoreOpToVkStoreOp(depthAttachmentDesc.storeOp);
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        if(depthAttachmentDesc.loadOp!=DeviceAttachmentLoadOp::DontCare)
        {
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else 
        {
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference& depthAttachmentRef= storage.depthAttachRef;
        depthAttachmentRef.attachment = attachments;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        ++attachments;
    }

    VkSubpassDescription& subpass = storage.subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = desc.colorAttachmentCount;
    if (desc.colorAttachmentCount)
    {
        subpass.pColorAttachments = storage.colorAttachRefs;
    }
    if (desc.depthAttachment)
    {
        subpass.pDepthStencilAttachment = &storage.depthAttachRef;
    }

    VkSubpassDependency& dependency = storage.dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo& renderPassInfo = storage.renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments;
    renderPassInfo.pAttachments = storage.attachs;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
}
} // namespace Aether