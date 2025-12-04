#include "Semaphore.h"
#include "GlobalRenderContext.h"
#include <iostream>
namespace Aether {
namespace vk {
std::optional<Semaphore> Semaphore::Create()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    if (vkCreateSemaphore(GRC::GetDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
    {
        return std::nullopt;
    }

    return Semaphore(semaphore);
}
Scope<Semaphore> Semaphore::CreateScope()
{
    auto opt = Create();
    if (!opt.has_value())
    {
        return nullptr;
    }
    return Aether::CreateScope<Semaphore>(std::move(opt.value()));
}
Semaphore::~Semaphore()
{
    if (m_Semaphore != VK_NULL_HANDLE)
        vkDestroySemaphore(GRC::GetDevice(), m_Semaphore, nullptr);
}
VkSemaphore Semaphore::GetHandle() const
{
    return m_Semaphore;
}

Semaphore::Semaphore(Semaphore&& other) noexcept
{
    m_Semaphore = other.m_Semaphore;
    other.m_Semaphore = VK_NULL_HANDLE;
}
Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
{
    if (this != &other)
    {
        if (m_Semaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(GRC::GetDevice(), m_Semaphore, nullptr);
        m_Semaphore = other.m_Semaphore;
        other.m_Semaphore = VK_NULL_HANDLE;
    }
    return *this;
}
//@breif
// 进行一次空的队列提交，信号这个 semaphore，阻塞cpu直到这个动作完成
// 仅debug用
void Semaphore::Signal()
{
#ifdef NDEBUG
    std::cerr << "Semaphore::Signal() is only for debug use\n"
              << std::endl;
    std::abort();
#endif
    Fence fenceOpt = Fence::Create().value();
    auto fence = std::move(fenceOpt);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0; // 我们不等待任何 semaphore
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 0; // 我们不提交任何 command buffer
    submitInfo.pCommandBuffers = nullptr;
    submitInfo.signalSemaphoreCount = 1;         // 我们要信号一个 semaphore
    submitInfo.pSignalSemaphores = &m_Semaphore; // 这是我们要信号的 semaphore
    vkQueueSubmit(GRC::GetGraphicsQueue().GetHandle(), 1, &submitInfo, fence.GetHandle());
    fence.Wait();
}
//@breif
// 进行一次空的队列提交，等待这个 semaphore，阻塞cpu直到这个动作完成
// 仅debug用
void Semaphore::Wait()
{
#ifdef NDEBUG
    std::cerr << "Semaphore::Wait() is only for debug use\n"
              << std::endl;
    std::abort();
#endif
    Fence fence = Fence::Create().value();
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_Semaphore;
    submitInfo.pWaitDstStageMask = &waitStages;
    submitInfo.commandBufferCount = 0;
    submitInfo.pCommandBuffers = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    vkQueueSubmit(GRC::GetGraphicsQueue().GetHandle(), 1, &submitInfo, fence.GetHandle());
    fence.Wait();
}
}} // namespace Aether::vk