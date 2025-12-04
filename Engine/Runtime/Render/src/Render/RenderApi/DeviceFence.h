#pragma once
#include <Render/Vulkan/Fence.h>
#include <variant>
#include <cassert>
namespace Aether
{
    class DeviceFence
    {
    public:
        vk::Fence& GetVk()
        {
            assert(std::holds_alternative<vk::Fence>(m_Fence));
            return std::get<vk::Fence>(m_Fence);
        }
        DeviceFence(vk::Fence&& fence)
            : m_Fence(std::move(fence))
        {
        }
        DeviceFence() = default;
        DeviceFence(const DeviceFence&) = delete;
        DeviceFence& operator=(const DeviceFence&) = delete;
        DeviceFence(DeviceFence&& other) noexcept
            : m_Fence(std::move(other.m_Fence))
        {
            other.m_Fence = std::monostate{};
        }
        DeviceFence& operator=(DeviceFence&& other) noexcept
        {
            if (this != &other)
            {
                m_Fence = std::move(other.m_Fence);
                other.m_Fence = std::monostate{};
            }
            return *this;
        }

    private:
        std::variant<std::monostate,vk::Fence> m_Fence;
    };
}