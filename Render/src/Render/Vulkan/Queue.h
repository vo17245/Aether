#pragma once
#include "Fence.h"
#include "Render/Vulkan/Semaphore.h"
#include <span>
#include "Traits.h"
#include "vulkan/vulkan_core.h"
#include "GraphicsCommandBuffer.h"
namespace Aether {
namespace vk {
class RenderContext;
class Queue
{
    friend class RenderContext;

public:
    enum class Type
    {
        Graphics,
        Compute,
        Transfer,
        Present,
        Unknown,
    };

public:
    class SubmitBuilder
    {
    public:
        SubmitBuilder(const Queue& queue) :
            m_Queue(queue)
        {
        }
        void BeginSubmitBatch();
        VkResult EndSubmitBatch();
        VkResult EndSubmitBatch(Fence& fence);
        VkResult EndSubmitBatch(VkFence fence);

        void BeginSubmit(std::span<GraphicsCommandBuffer> commandBuffers);
        void BeginSubmit(GraphicsCommandBuffer& commandBuffer);

        void Signal(std::span<Semaphore> semaphores);
        void Signal(Semaphore semaphore);
        void Signal(std::span<VkSemaphore> semaphores);
        void Signal(VkSemaphore semaphore);

        void Wait(std::span<Semaphore> semaphores, std::span<VkPipelineStageFlags> waitStages);
        void Wait(Semaphore semaphore, VkPipelineStageFlags waitStage);
        void Wait(std::span<VkSemaphore> semaphores, std::span<VkPipelineStageFlags> waitStages);
        void Wait(VkSemaphore semaphore, VkPipelineStageFlags waitStage);

    private:
        void OnBeginSubmit();
        const Queue& m_Queue;
        std::vector<std::vector<VkCommandBuffer>> m_CommandBuffers;
        std::vector<std::vector<VkSemaphore>> m_Waits;
        std::vector<std::vector<VkPipelineStageFlags>> m_WaitStages;
        std::vector<std::vector<VkSemaphore>> m_Signals;
    };
    VkQueue GetHandle() const
    {
        return m_Handle;
    }
    bool operator()() const
    {
        return m_Handle != VK_NULL_HANDLE;
    }

private:
    Queue(VkQueue handle, Type type) :
        m_Handle(handle), m_Type(type)
    {
    }
    Queue() :
        m_Handle(VK_NULL_HANDLE), m_Type(Type::Unknown)
    {
    }
    VkQueue m_Handle;
    Type m_Type;
};
}
} // namespace Aether::vk
