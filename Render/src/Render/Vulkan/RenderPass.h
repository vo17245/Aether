#pragma once
#include <optional>
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {

class RenderPass
{
public:
    static std::optional<RenderPass> CreateForPresent(VkFormat colorAttachmentFormat);

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
    static std::optional<VkRenderPass> createRenderPass(VkFormat colorAttachmentFormat);
};
}
} // namespace Aether::vk