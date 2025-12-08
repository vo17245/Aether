#include "SubmitThread.h"
#include <Debug/Log.h>
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
SubmitThread& SubmitThread::GetSingleton()
{
    static SubmitThread instance;
    return instance;
}
void SubmitThread::InitImpl()
{
    m_Running = true;
    m_Thread.emplace(Worker{*this});
}
void SubmitThread::ShutdownImpl()
{
    m_Running = false;
    m_SubmitCondition.notify_all();
    if (m_Thread && m_Thread->joinable())
    {
        m_Thread->join();
    }
}
static void HandleVkCommandSubmit(const CommandSubmit& submit)
{
    std::vector<VkSemaphore> waitSemaphores;
    for (auto* semaphore : submit.waitSemaphores)
    {
        waitSemaphores.push_back(semaphore->GetVk().GetHandle());
    }
    std::vector<VkPipelineStageFlags> waitStages;
    for (auto stage : submit.waitStages)
    {
        waitStages.push_back(DevicePipelineSyncStageToVk(stage));
    }
    std::vector<VkSemaphore> signalSemaphores;
    for (auto* semaphore : submit.signalSemaphores)
    {
        signalSemaphores.push_back(semaphore->GetVk().GetHandle());
    }
    VkCommandBuffer vkCmdBuffer = submit.commandBuffer->GetVk().GetHandle();
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
        vkFence = submit.signalFence->GetVk().GetHandle();
    }
    vkQueueSubmit(submit.queue.GetVk()->GetHandle(), 1, &submitInfo, vkFence);
}
static void HandleVkPresentSubmit(const PresentSubmit& submit)
{
    std::vector<VkSemaphore> waitSemaphores;
    for (auto* semaphore : submit.waitSemaphores)
    {
        waitSemaphores.push_back(semaphore->GetVk().GetHandle());
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = submit.waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();
    VkSwapchainKHR vkSwapChain = submit.swapChain->GetVk().GetHandle();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkSwapChain;
    presentInfo.pImageIndices = &submit.imageIndex;
    vkQueuePresentKHR(vk::GRC::GetPresentQueue().GetHandle(), &presentInfo);
}
static void HandleVkImageAcquireSubmit(const ImageAcquireSubmit& submit)
{
    const ImageAcquireSubmit& imageAcquireSubmit = static_cast<const ImageAcquireSubmit&>(submit);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        vk::GRC::GetDevice(), imageAcquireSubmit.swapChain->GetVk().GetHandle(), imageAcquireSubmit.timeoutNs,
        imageAcquireSubmit.signalSemaphore->GetVk().GetHandle(), VK_NULL_HANDLE, &imageIndex);
    if (imageAcquireSubmit.result)
    {
        ImageAcquireResult acquireResult;
        acquireResult.imageIndex = imageIndex;
        switch (result)
        {
        case VK_NOT_READY:
            acquireResult.status = ImageAcquireStatus::NotReady;
            break;
        case VK_TIMEOUT:
            acquireResult.status = ImageAcquireStatus::Timeout;
            break;
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
    case SubmitType::Command: {
        const CommandSubmit& commandSubmit = static_cast<const CommandSubmit&>(submit);
        HandleVkCommandSubmit(commandSubmit);
        break;
    }
    case SubmitType::Present: {
        const PresentSubmit& presentSubmit = static_cast<const PresentSubmit&>(submit);
        HandleVkPresentSubmit(presentSubmit);
        break;
    }
    case SubmitType::ImageAcquire: {
        const ImageAcquireSubmit& imageAcquireSubmit = static_cast<const ImageAcquireSubmit&>(submit);
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
            if (thread.m_Running == false && !thread.m_Queue.size()>0)
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
}
} // namespace Aether::Render