#include "Fence.h"
#include "GlobalRenderContext.h"

namespace Aether {
namespace vk {
std::optional<Fence> Fence::Create(bool signaled)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFenceCreateFlags flags = 0;
    if (signaled)
        flags |= VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.flags = flags;
    VkFence fence;
    if (vkCreateFence(GRC::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        return std::nullopt;
    }

    return Fence(fence);
}
std::unique_ptr<Fence> Fence::CreateScope(bool signaled)
{
    auto* res = CreatePtr(signaled);
    if (!res) return nullptr;
    return std::unique_ptr<Fence>(res);
}
Fence* Fence::CreatePtr(bool signaled)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFenceCreateFlags flags = 0;
    if (signaled)
        flags |= VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.flags = flags;
    VkFence fence;
    if (vkCreateFence(GRC::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        return nullptr;
    }
    Fence* res = new Fence(fence);

    return res;
}
Fence::~Fence()
{
    if (m_Fence != VK_NULL_HANDLE)
        vkDestroyFence(GRC::GetDevice(), m_Fence, nullptr);
}
VkFence Fence::GetHandle() const
{
    return m_Fence;
}

Fence::Fence(Fence&& other) noexcept
{
    m_Fence = other.m_Fence;
    other.m_Fence = VK_NULL_HANDLE;
}
Fence& Fence::operator=(Fence&& other) noexcept
{
    if (this != &other)
    {
        if (m_Fence != VK_NULL_HANDLE)
            vkDestroyFence(GRC::GetDevice(), m_Fence, nullptr);
        m_Fence = other.m_Fence;
        other.m_Fence = VK_NULL_HANDLE;
    }
    return *this;
}
void Fence::Reset()
{
    vkResetFences(GRC::GetDevice(), 1, &m_Fence);
}
//@return
//  Result::None ok
//  Result::Timeout timeout
VkResult Fence::Wait(uint64_t timeout)
{
    return vkWaitForFences(GRC::GetDevice(), 1, &m_Fence, VK_TRUE, timeout);
}
}
} // namespace Aether::vk