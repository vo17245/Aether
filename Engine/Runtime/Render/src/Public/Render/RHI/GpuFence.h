#pragma once
#include "Backend/Vulkan/Fence.h"
#include <variant>
#include <cassert>
namespace Aether::rhi
{
    class GpuFence
    {
    public:
        GpuFence(vk::Fence&& fence)
            : m_Fence(std::move(fence))
        {
        }
        GpuFence() = default;
        GpuFence(const GpuFence&) = delete;
        GpuFence& operator=(const GpuFence&) = delete;
        GpuFence(GpuFence&& other) noexcept
            : m_Fence(std::move(other.m_Fence))
        {
            other.m_Fence = std::monostate{};
        }
        GpuFence& operator=(GpuFence&& other) noexcept
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