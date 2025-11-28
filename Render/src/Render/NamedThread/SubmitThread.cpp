#include "SubmitThread.h"

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
    m_Thread.emplace(Worker{*this, m_CommandBufferQueue});
    m_Dispatcher.emplace(Dispatcher{.graphicsQueue = m_CommandBufferQueue,
                                    .transferQueue = m_CommandBufferQueue,
                                    .computeQueue = m_CommandBufferQueue});
}
void SubmitThread::ShutdownImpl()
{
    m_Running = false;
    m_Condition.notify_all();
    if (m_Thread && m_Thread->joinable())
    {
        m_Thread->join();
    }
}
void SubmitThread::Worker::operator()()
{
    for (;;)
    {
        Submit submit;
        {
            std::unique_lock<std::mutex> lock(thread.m_Mutex);
            thread.m_Condition.wait(lock, [&] { return !(queue.size_approx() == 0) || thread.m_Running == false; });
            if (thread.m_Running == false && queue.size_approx() == 0)
            {
                break;
            }
            queue.try_dequeue(submit);
        }
        assert(submit.commandBuffer != nullptr);
        switch(Render::Config::RenderApi)
        {
            case Render::Api::Vulkan:
            {
                std::vector<VkSemaphore> waitSemaphores;
                for(auto* semaphore:submit.waitSemaphores)
                {
                    waitSemaphores.push_back(semaphore->GetVk().GetHandle());
                }
                std::vector<VkPipelineStageFlags> waitStages;
                for(auto stage:submit.waitStages)
                {
                    waitStages.push_back(DevicePipelineSyncStageToVk(stage));
                }
                std::vector<VkSemaphore> signalSemaphores;
                for(auto* semaphore:submit.signalSemaphores)
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
                if(submit.signalFence)
                {
                    vkFence = submit.signalFence->GetVk().GetHandle();
                }
                vkQueueSubmit(vk::GRC::GetGraphicsQueue().GetHandle(), 1, &submitInfo, vkFence);
                break;
            }
            default:
            {
                assert(false && "Unsupported Render Api");
                break;
            }
        }
    }
}
} // namespace Aether::Render