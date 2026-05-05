#pragma once
#include "Backend/Vulkan/SwapChain.h"
#include <variant>
namespace Aether::rhi
{
    class SwapChain
    {
    public:
        SwapChain() = default;
        ~SwapChain() = default;
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain(SwapChain&& other) noexcept = default;
        SwapChain& operator=(SwapChain&& other) noexcept = default;
        SwapChain(vk::SwapChain&& SwapChain) : m_SwapChain(std::move(SwapChain)) {}
        operator bool() const
        {
            return !std::holds_alternative<std::monostate>(m_SwapChain);
        }
        vk::SwapChain& GetVk()
        {
            return std::get<vk::SwapChain>(m_SwapChain);
        }
    private:
        std::variant<std::monostate,vk::SwapChain> m_SwapChain;
    };
}