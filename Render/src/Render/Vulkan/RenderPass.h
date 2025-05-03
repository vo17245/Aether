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
    static std::optional<RenderPass> CreateForPresent(VkFormat colorAttachmentFormat);
    /**
     * @brief create a default render pass
     *    load op: clear
     *    color attachment format: rgba8_linear unorm(0-255)
     */
    static std::optional<RenderPass> CreateDefault(PixelFormat format=PixelFormat::RGBA8888);

    ~RenderPass();
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;
    VkRenderPass GetHandle() const;
    RenderPass(RenderPass&& other) noexcept;
    RenderPass& operator=(RenderPass&& other) noexcept;
    RenderPass(VkRenderPass renderPass);

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