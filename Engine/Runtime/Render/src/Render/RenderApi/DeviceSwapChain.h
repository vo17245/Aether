#pragma once
#include <variant>
#include "Render/Vulkan/SwapChain.h"
#include <cassert>
namespace Aether
{
    class DeviceSwapChain
    {
    public:
        DeviceSwapChain() = default;
        DeviceSwapChain(const DeviceSwapChain&) = delete;
        DeviceSwapChain(DeviceSwapChain&& other) = default;
        DeviceSwapChain& operator=(const DeviceSwapChain&) = delete;
        DeviceSwapChain& operator=(DeviceSwapChain&& other) = default;
        ~DeviceSwapChain() = default;
        DeviceSwapChain(vk::SwapChain&& swapChain)
            : m_SwapChain(std::move(swapChain))
        {
        }
    public:
        vk::SwapChain& GetVk()
        {
            assert(std::holds_alternative<vk::SwapChain>(m_SwapChain));
            return std::get<vk::SwapChain>(m_SwapChain);
        }
    private:
        std::variant<std::monostate,vk::SwapChain> m_SwapChain;
    };
}