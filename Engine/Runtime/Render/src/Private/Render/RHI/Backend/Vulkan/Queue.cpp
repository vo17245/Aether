#include "Render/RHI/Backend/Vulkan/Queue.h"
#include "vulkan/vulkan_core.h"
#include "Render/RHI/Backend/Vulkan/GraphicsCommandBuffer.h"

namespace Aether {
namespace vk {
void Queue::SubmitBuilder::BeginSubmitBatch()
{
}
VkResult Queue::SubmitBuilder::EndSubmitBatch()
{
    return EndSubmitBatch(VK_NULL_HANDLE);
}
VkResult Queue::SubmitBuilder::EndSubmitBatch(Fence& fence)
{
    return EndSubmitBatch(fence.GetHandle());
}
VkResult Queue::SubmitBuilder::EndSubmitBatch(VkFence fence)
{
    std::vector<VkSubmitInfo> submitInfos;
    submitInfos.reserve(m_CommandBuffers.size());
    for (size_t i = 0; i < m_CommandBuffers.size(); ++i)
    {
        auto& buffers = m_CommandBuffers[i];
        auto& waits = m_Waits[i];
        auto& waitStages = m_WaitStages[i];
        auto& signals = m_Signals[i];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = waits.size();
        submitInfo.pWaitSemaphores = waits.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = buffers.size();
        submitInfo.pCommandBuffers = buffers.data();

        submitInfo.signalSemaphoreCount = signals.size();
        submitInfo.pSignalSemaphores = signals.data();
        submitInfos.emplace_back(submitInfo);
    }

    return vkQueueSubmit(m_Queue.GetHandle(),
                         submitInfos.size(),
                         submitInfos.data(),
                         fence);
}
void Queue::SubmitBuilder::BeginSubmit(std::span<GraphicsCommandBuffer> commandBuffers)
{
    OnBeginSubmit();
    for (auto& commandBuffer : commandBuffers)
    {
        m_CommandBuffers.back().emplace_back(commandBuffer.GetHandle());
    }
}
void Queue::SubmitBuilder::BeginSubmit(GraphicsCommandBuffer& commandBuffer)
{
    OnBeginSubmit();
    m_CommandBuffers.back().emplace_back(commandBuffer.GetHandle());
}
void Queue::SubmitBuilder::Signal(std::span<Semaphore> semaphores)
{
    for (auto& semaphore : semaphores)
    {
        m_Signals.back().emplace_back(semaphore.GetHandle());
    }
}
void Queue::SubmitBuilder::Signal(Semaphore semaphore)
{
    m_Signals.back().emplace_back(semaphore.GetHandle());
}
void Queue::SubmitBuilder::Signal(std::span<VkSemaphore> semaphores)
{
    for (auto& semaphore : semaphores)
    {
        m_Signals.back().emplace_back(semaphore);
    }
}
void Queue::SubmitBuilder::Signal(VkSemaphore semaphore)
{
    m_Signals.back().emplace_back(semaphore);
}
void Queue::SubmitBuilder::Wait(std::span<Semaphore> semaphores, std::span<VkPipelineStageFlags> waitStages)
{
    for (auto& semaphore : semaphores)
    {
        m_Waits.back().emplace_back(semaphore.GetHandle());
    }

    for (auto& waitStage : waitStages)
    {
        m_WaitStages.back().emplace_back(waitStage);
    }
}
void Queue::SubmitBuilder::Wait(Semaphore semaphore, VkPipelineStageFlags waitStage)
{
    m_Waits.back().emplace_back(semaphore.GetHandle());
    m_WaitStages.back().emplace_back(waitStage);
}
void Queue::SubmitBuilder::Wait(std::span<VkSemaphore> semaphores, std::span<VkPipelineStageFlags> waitStages)
{
    for (auto& semaphore : semaphores)
    {
        m_Waits.back().emplace_back(semaphore);
    }
    for (auto& waitStage : waitStages)
    {
        m_WaitStages.back().emplace_back(waitStage);
    }
}
void Queue::SubmitBuilder::Wait(VkSemaphore semaphore, VkPipelineStageFlags waitStage)
{
    m_Waits.back().emplace_back(semaphore);
    m_WaitStages.back().emplace_back(waitStage);
}
void Queue::SubmitBuilder::OnBeginSubmit()
{
    m_Waits.emplace_back();
    m_WaitStages.emplace_back();
    m_CommandBuffers.emplace_back();
    m_Signals.emplace_back();
}

}
} // namespace Aether::vk