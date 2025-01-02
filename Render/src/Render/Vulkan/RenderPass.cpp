#include "RenderPass.h"
#include "GlobalRenderContext.h"

namespace Aether {
namespace vk {
std::optional<RenderPass> RenderPass::CreateForPresent(VkFormat colorAttachmentFormat)
{
    RenderPass renderPass;
    auto res = createRenderPass(colorAttachmentFormat);
    if (!res.has_value())
    {
        return std::nullopt;
    }
    renderPass.m_RenderPass = res.value();
    return renderPass;
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(GlobalRenderContext::GetDevice(), m_RenderPass, nullptr);
}

VkRenderPass RenderPass::GetHandle() const
{
    return m_RenderPass;
}
RenderPass::RenderPass(RenderPass&& other) noexcept
{
    if (m_RenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(GlobalRenderContext::GetDevice(), m_RenderPass, nullptr);
    }
    m_RenderPass = other.m_RenderPass;
    other.m_RenderPass = VK_NULL_HANDLE;
}
RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
{
    if (m_RenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(GlobalRenderContext::GetDevice(), m_RenderPass, nullptr);
    }
    m_RenderPass = other.m_RenderPass;
    other.m_RenderPass = VK_NULL_HANDLE;
    return *this;
}
RenderPass::RenderPass(VkRenderPass renderPass) :
    m_RenderPass(renderPass)
{
}

std::optional<VkRenderPass> RenderPass::createRenderPass(VkFormat colorAttachmentFormat)
{
    VkRenderPass renderPass;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = colorAttachmentFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(GlobalRenderContext::GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return renderPass;
}
}
} // namespace Aether::vk