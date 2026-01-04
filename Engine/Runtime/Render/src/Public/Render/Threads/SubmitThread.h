#pragma once
#include <Core/Core.h>
#include <Render/RHI.h>
#include <semaphore>
#include <queue>
#include <Render/RHI/Backend/Vulkan/Queue.h>
#include <Render/RHI/Backend/Vulkan/SwapChain.h>


namespace Aether::Render
{
enum class SubmitType
{
    VkCommand,
    VkPresent,
    VkImageAcquire,
    Custom,
};
struct SubmitBase
{
    SubmitType type;
    SubmitBase(SubmitType t) : type(t)
    {
    }
    virtual ~SubmitBase() = default;
};
enum class QueueType
{
    Graphics,
    Compute,
    Transfer,
    Present,
    Unknown,
};
enum class PipelineSyncStage
{
    AllGraphics,
};
inline constexpr VkPipelineStageFlags DevicePipelineSyncStageToVk(PipelineSyncStage stage)
{
    switch (stage)
    {
    case PipelineSyncStage::AllGraphics:
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    default:
        assert(false && "Not implemented");
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
}

struct VkCommandSubmit : SubmitBase
{
    vk::GraphicsCommandBuffer* commandBuffer = nullptr;
    vk::Fence* signalFence = nullptr;
    vk::Queue* queue;
    std::vector<PipelineSyncStage> waitStages;
    std::vector<vk::Semaphore*> waitSemaphores;
    std::vector<vk::Semaphore*> signalSemaphores;
};
struct VkPresentSubmit : SubmitBase
{
    VkPresentSubmit() : SubmitBase(SubmitType::VkPresent)
    {
    }
    vk::SwapChain* swapChain = nullptr;
    std::vector<vk::Semaphore*> waitSemaphores;
    uint32_t imageIndex = 0;
};
enum class ImageAcquireStatus
{
    Success,
    NotReady,
    Timeout,
    Error,
};
struct ImageAcquireResult
{
    ImageAcquireStatus status;
    uint32_t imageIndex;
};
struct VkImageAcquireSubmit : SubmitBase
{
    VkImageAcquireSubmit() : SubmitBase(SubmitType::VkImageAcquire)
    {
    }
    vk::SwapChain* swapChain = nullptr;
    vk::Semaphore* signalSemaphore = nullptr;
    uint64_t timeoutNs = UINT64_MAX;
    ImageAcquireResult* result = nullptr;
    std::binary_semaphore* semaphore = nullptr;
};
struct CustomSubmit : SubmitBase
{
    CustomSubmit() : SubmitBase(SubmitType::Custom)
    {
    }
    std::function<void()> func;
    std::binary_semaphore* semaphore = nullptr;
};

class SubmitThread
{
public:
    static void Init();
    static void Shutdown();

    static void PushSubmit(Scope<SubmitBase>&& submit)
    {
        std::lock_guard lock(GetSingleton().m_SubmitMutex);
        GetSingleton().m_Queue.push(std::move(submit));
        GetSingleton().m_SubmitCondition.notify_one();
    }

private:
    static SubmitThread& GetSingleton();
    void InitImpl();
    void ShutdownImpl();
    struct Worker
    {
        SubmitThread& thread;
        void operator()();
    };

    std::optional<std::thread> m_Thread;
    std::atomic<bool> m_Running = false;
    std::mutex m_SubmitMutex;
    std::condition_variable m_SubmitCondition;

private:
    std::queue<std::unique_ptr<SubmitBase>> m_Queue;
};
} // namespace Aether::Render