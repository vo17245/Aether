#pragma once
#include "vulkan/vulkan_core.h"
namespace Aether::vk
{
    class SwapChain
    {
    public:
        SwapChain(VkSwapchainKHR swapChain)
            : m_SwapChain(swapChain)
        {
        }
        
        ~SwapChain();
        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&& other) 
        {
            m_SwapChain = other.m_SwapChain;
            other.m_SwapChain = VK_NULL_HANDLE;
        }
        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain& operator=(SwapChain&& other) 
        {
            if (this != &other)
            {
                Destroy();
                m_SwapChain = other.m_SwapChain;
                other.m_SwapChain = VK_NULL_HANDLE;
            }
            return *this;
        }

    public:
        VkSwapchainKHR GetHandle() const
        {
            return m_SwapChain;
        }

    private:
        void Destroy();
        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
    };
}