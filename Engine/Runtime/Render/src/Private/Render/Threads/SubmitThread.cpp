#include "Render/Threads/SubmitThread.h"
#include <Debug/Log.h>
#include <Render/RHI/Backend/Vulkan/GlobalRenderContext.h>
namespace Aether::Render
{
void SubmitThread::Init()
{
    GetSingleton().InitImpl();
}

void SubmitThread::Shutdown()
{
    GetSingleton().ShutdownImpl();
}
bool SubmitThread::WaitIdle()
{
    auto& instance = GetSingleton();
    {
        std::lock_guard lock(instance.m_SubmitMutex);
        if (instance.m_WorkerId == std::this_thread::get_id())
        {
            return false;
        }
        if (!instance.m_Running) return false;
    }
    std::binary_semaphore completed{0};
    auto submit = CreateScope<CustomSubmit>();
    submit->func = []() { vkDeviceWaitIdle(vk::GRC::GetDevice()); };
    submit->semaphore = &completed;
    if (!PushSubmit(std::move(submit))) return false;
    completed.acquire();
    return true;
}
bool SubmitThread::IsRunning()
{
    return GetSingleton().m_Running.load();
}
SubmitThread& SubmitThread::GetSingleton()
{
    static SubmitThread instance;
    return instance;
}
void SubmitThread::InitImpl()
{
    std::lock_guard lock(m_SubmitMutex);
    if (m_Running) return;
    if (m_Thread && m_Thread->joinable()) return;
    m_Running = true;
    m_Thread.emplace(Worker{*this});
}
void SubmitThread::ShutdownImpl()
{
    {
        std::lock_guard lock(m_SubmitMutex);
        if (!m_Running && (!m_Thread || !m_Thread->joinable())) return;
        m_Running = false;
    }
    m_SubmitCondition.notify_all();
    if (m_Thread && m_Thread->joinable())
    {
        if (m_Thread->get_id() == std::this_thread::get_id()) return;
        m_Thread->join();
    }
    m_Thread.reset();
}
static void HandleVkCommandSubmit(const VkCommandSubmit& submit)
{
    std::vector<VkSemaphore> waitSemaphores;
    for (auto* semaphore : submit.waitSemaphores)
    {
        waitSemaphores.push_back(semaphore->GetHandle());
    }
    std::vector<VkPipelineStageFlags> waitStages;
    for (auto stage : submit.waitStages)
    {
        waitStages.push_back(DevicePipelineSyncStageToVk(stage));
    }
    std::vector<VkSemaphore> signalSemaphores;
    for (auto* semaphore : submit.signalSemaphores)
    {
        signalSemaphores.push_back(semaphore->GetHandle());
    }
    VkCommandBuffer vkCmdBuffer = submit.commandBuffer->GetHandle();
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = submit.waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCmdBuffer;
    submitInfo.signalSemaphoreCount = submit.signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    VkFence vkFence = VK_NULL_HANDLE;
    if (submit.signalFence)
    {
        vkFence = submit.signalFence->GetHandle();
    }
    vkQueueSubmit(submit.queue->GetHandle(), 1, &submitInfo, vkFence);
}
static void HandleVkPresentSubmit(const VkPresentSubmit& submit)
{
    std::vector<VkSemaphore> waitSemaphores;
    for (auto* semaphore : submit.waitSemaphores)
    {
        waitSemaphores.push_back(semaphore->GetHandle());
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = submit.waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();
    VkSwapchainKHR vkSwapChain = submit.swapChain->GetHandle();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkSwapChain;
    presentInfo.pImageIndices = &submit.imageIndex;
    vkQueuePresentKHR(vk::GRC::GetPresentQueue().GetHandle(), &presentInfo);
}
static void HandleVkImageAcquireSubmit(const VkImageAcquireSubmit& submit)
{
    const VkImageAcquireSubmit& imageAcquireSubmit = static_cast<const VkImageAcquireSubmit&>(submit);
    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(
        vk::GRC::GetDevice(), imageAcquireSubmit.swapChain->GetHandle(), imageAcquireSubmit.timeoutNs,
        imageAcquireSubmit.signalSemaphore->GetHandle(), VK_NULL_HANDLE, &imageIndex);
    if (imageAcquireSubmit.result)
    {
        ImageAcquireResult acquireResult;
        acquireResult.imageIndex = imageIndex;
        switch (result)
        {
        case VK_ERROR_OUT_OF_DATE_KHR:
            acquireResult.status = ImageAcquireStatus::OutOfDate;
            break;
        case VK_NOT_READY:
            acquireResult.status = ImageAcquireStatus::NotReady;
            break;
        case VK_TIMEOUT:
            acquireResult.status = ImageAcquireStatus::Timeout;
            break;
        case VK_SUBOPTIMAL_KHR:
        case VK_SUCCESS:
            acquireResult.status = ImageAcquireStatus::Success;
            break;
        
        default:
            acquireResult.status = ImageAcquireStatus::Error;
            break;
        }
        *imageAcquireSubmit.result=acquireResult;
        if(submit.semaphore)
        {
            submit.semaphore->release();
        }
    }
}
static void HandleVkSubmit(const SubmitBase& submit)
{
    switch (submit.type)
    {
    case SubmitType::VkCommand: {
        const VkCommandSubmit& commandSubmit = static_cast<const VkCommandSubmit&>(submit);
        HandleVkCommandSubmit(commandSubmit);
        break;
    }
    case SubmitType::VkPresent: {
        const VkPresentSubmit& presentSubmit = static_cast<const VkPresentSubmit&>(submit);
        HandleVkPresentSubmit(presentSubmit);
        break;
    }
    case SubmitType::VkImageAcquire: {
        const VkImageAcquireSubmit& imageAcquireSubmit = static_cast<const VkImageAcquireSubmit&>(submit);
        HandleVkImageAcquireSubmit(imageAcquireSubmit);
        break;
    }
    case SubmitType::Custom:{
        const CustomSubmit& customSubmit = static_cast<const CustomSubmit&>(submit);
        if(customSubmit.func)
        {
            customSubmit.func();
        }
        if(customSubmit.semaphore)
        {
            customSubmit.semaphore->release();
        }
        break;
    }
    default: {
        assert(false && "Unknown Submit Type");
        break;
    }
    }
}
void SubmitThread::Worker::operator()()
{
    {
        std::lock_guard lock(thread.m_SubmitMutex);
        thread.m_WorkerId = std::this_thread::get_id();
    }
    for (;;)
    {
        std::unique_ptr<SubmitBase> submit;
        {
            std::unique_lock<std::mutex> lock(thread.m_SubmitMutex);
            thread.m_SubmitCondition.wait(
                lock, [&] 
                { 
                    return thread.m_Queue.size()>0 || thread.m_Running == false; 
                });
            if (!thread.m_Running && thread.m_Queue.empty())
            {
                break;
            }
            submit = std::move(thread.m_Queue.front());
            thread.m_Queue.pop();
        }
        assert(submit != nullptr);
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            HandleVkSubmit(*submit);
            break;
        }
        default: {
            assert(false && "Unsupported Render Api");
            break;
        }
        }
    }
    {
        std::lock_guard lock(thread.m_SubmitMutex);
        thread.m_WorkerId = {};
    }
}
} // namespace Aether::Render
