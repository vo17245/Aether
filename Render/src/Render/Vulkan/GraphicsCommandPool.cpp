#include "GraphicsCommandPool.h"
#include "GlobalRenderContext.h"

namespace Aether {
namespace vk {
std::optional<GraphicsCommandPool> GraphicsCommandPool::Create()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    assert(GRC::GetQueueFamilyIndices().graphicsFamily.has_value());
    poolInfo.queueFamilyIndex = GRC::GetQueueFamilyIndices().graphicsFamily.value();
    VkCommandPool commandPool;
    if (vkCreateCommandPool(GRC::GetDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return GraphicsCommandPool(commandPool);
}
Scope<GraphicsCommandPool> GraphicsCommandPool::CreateScope()
{
    auto commandPool = Create();
    if (!commandPool.has_value())
    {
        return nullptr;
    }
    return Aether::CreateScope<GraphicsCommandPool>(std::move(commandPool.value()));
}
GraphicsCommandPool::GraphicsCommandPool(GraphicsCommandPool&& other) noexcept
{
    m_Handle = other.m_Handle;
    other.m_Handle = VK_NULL_HANDLE;
}
GraphicsCommandPool& GraphicsCommandPool::operator=(GraphicsCommandPool&& other) noexcept
{
    if (this != &other)
    {
        if (m_Handle != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(GRC::GetDevice(), m_Handle, nullptr);
        }
        m_Handle = other.m_Handle;
        other.m_Handle = VK_NULL_HANDLE;
    }
    return *this;
}
GraphicsCommandPool::~GraphicsCommandPool()
{
    if (m_Handle != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(GRC::GetDevice(), m_Handle, nullptr);
    }
}
VkCommandPool GraphicsCommandPool::GetHandle() const
{
    return m_Handle;
}
}
} // namespace Aether::vk