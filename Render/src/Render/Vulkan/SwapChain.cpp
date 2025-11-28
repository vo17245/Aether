#include "SwapChain.h"
#include "Render/Vulkan/GlobalRenderContext.h"
namespace Aether::vk
{
SwapChain::~SwapChain()
{
    Destroy();
}

void SwapChain::Destroy()
{
    if (m_SwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(GRC::GetDevice(), m_SwapChain, nullptr);
        m_SwapChain = VK_NULL_HANDLE;
    }
}
} // namespace Aether::vk