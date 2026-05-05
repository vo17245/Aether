#pragma once
#include "Backend/Vulkan/Fence.h"
#include <variant>
#include <cassert>
#include "Backend/Vulkan/Semaphore.h"
namespace Aether::rhi
{
namespace Detail
{
struct VulkanSync
{
    std::optional<vk::Fence> fence;
    std::optional<vk::Semaphore> semaphore;
};
} // namespace Detail
class Fence
{
public:
    Fence(vk::Fence&& fence) : m_Fence(Detail::VulkanSync{std::move(fence), std::nullopt})
    {
    }
    Fence(vk::Semaphore&& semaphore) : m_Fence(Detail::VulkanSync{std::nullopt, std::move(semaphore)})
    {
    }
    Fence() = default;
    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;
    Fence(Fence&& other) noexcept : m_Fence(std::move(other.m_Fence))
    {
        other.m_Fence = std::monostate{};
    }
    Fence& operator=(Fence&& other) noexcept
    {
        if (this != &other)
        {
            m_Fence = std::move(other.m_Fence);
            other.m_Fence = std::monostate{};
        }
        return *this;
    }
    operator bool() const
    {
        return !std::holds_alternative<std::monostate>(m_Fence);
    }
    vk::Fence& GetVkFence()
    {
        assert(std::holds_alternative<Detail::VulkanSync>(m_Fence)
               && std::get<Detail::VulkanSync>(m_Fence).fence.has_value());
        return std::get<Detail::VulkanSync>(m_Fence).fence.value();
    }
    vk::Semaphore& GetVkSemaphore()
    {
        assert(std::holds_alternative<Detail::VulkanSync>(m_Fence)
               && std::get<Detail::VulkanSync>(m_Fence).semaphore.has_value());
        return std::get<Detail::VulkanSync>(m_Fence).semaphore.value();
    }

private:
    std::variant<std::monostate, Detail::VulkanSync> m_Fence;
};
} // namespace Aether::rhi