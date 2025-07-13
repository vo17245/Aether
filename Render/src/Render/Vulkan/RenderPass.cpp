#include "RenderPass.h"
#include "GlobalRenderContext.h"
#include "Render/PixelFormat.h"
#include "vulkan/vulkan_core.h"

namespace Aether::vk
{
std::optional<RenderPass> RenderPass::CreateForPresent(VkFormat colorAttachmentFormat)
{
    RenderPass renderPass;
    auto res = CreateRenderPass(colorAttachmentFormat);
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

std::optional<VkRenderPass> RenderPass::CreateRenderPass(VkFormat colorAttachmentFormat)
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
std::optional<RenderPass> RenderPass::CreateDefault(PixelFormat format)
{
    RenderPass renderPass;
    auto res = CreateDefaultImpl(PixelFormatToVkFormat(format));
    if (!res.has_value())
    {
        return std::nullopt;
    }
    renderPass.m_RenderPass = res.value();
    return renderPass;
}
std::optional<VkRenderPass> RenderPass::CreateDefaultImpl(VkFormat format)
{
    VkRenderPass renderPass;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
std::optional<RenderPass> RenderPass::CreateForDepthTest()
{
    VkRenderPass renderPass;
    VkAttachmentDescription attachments[2]{};
    // config color attachment
    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // config depth attachment
    VkAttachmentDescription& depthAttachment = attachments[1];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
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
std::optional<RenderPass> RenderPass::Create(const VkRenderPassCreateInfo& info)
{
    VkRenderPass renderPass;
    if (vkCreateRenderPass(GlobalRenderContext::GetDevice(), &info, nullptr, &renderPass) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return renderPass;
}
} // namespace Aether::vk