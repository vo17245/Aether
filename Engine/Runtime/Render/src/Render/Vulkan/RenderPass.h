#pragma once
#include <optional>
#include "Render/PixelFormat.h"
#include "vulkan/vulkan_core.h"
namespace Aether
{
namespace vk
{
class RenderPass
{
public:
    ~RenderPass();
    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&& other) noexcept;
    RenderPass& operator=(RenderPass&& other) noexcept;
    RenderPass(VkRenderPass renderPass);

public:
    static std::optional<RenderPass> Create(const VkRenderPassCreateInfo& info);
    static std::optional<RenderPass> CreateForPresent(VkFormat colorAttachmentFormat);
    /**
     * @brief create a default render pass
     *    load op: clear
     *    color attachment format: param format
     */
    static std::optional<RenderPass> CreateDefault(PixelFormat format = PixelFormat::RGBA8888);
    /**
     * @brief create a render pass
     * load op: clear
     * color attachment(attach 0) format: rgba8888 linear unorm(0-255)
     * depth attachment(attach 1) format: depth32
     */
    static std::optional<RenderPass> CreateForDepthTest();

    VkRenderPass GetHandle() const;

private:
    RenderPass() = default;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    /**
     * @brief create for present
     */
    static std::optional<VkRenderPass> CreateRenderPass(VkFormat colorAttachmentFormat);
    static std::optional<VkRenderPass> CreateDefaultImpl(VkFormat format);
};
} // namespace vk
} // namespace Aether