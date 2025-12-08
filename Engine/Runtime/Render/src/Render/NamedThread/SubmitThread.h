#pragma once
#include <Core/Core.h>
#include <Render/RenderApi.h>
#include <semaphore>
#include <queue>
namespace Aether::Render
{
enum class SubmitType
{
    Command,
    Present,
    ImageAcquire,
    Custom,
};
struct SubmitBase
{
    SubmitType type;
    SubmitBase(SubmitType t) : type(t) {}
    virtual ~SubmitBase() = default;
};
struct CommandSubmit: SubmitBase
{
    CommandSubmit() : SubmitBase(SubmitType::Command) {}
    DeviceCommandBuffer* commandBuffer=nullptr;
    DeviceFence* signalFence=nullptr;
    DeviceQueueView queue;
    std::vector<DevicePipelineSyncStage> waitStages;
    std::vector<DeviceSemaphore*> waitSemaphores;
    std::vector<DeviceSemaphore*> signalSemaphores;
};
struct PresentSubmit: SubmitBase
{
    PresentSubmit() : SubmitBase(SubmitType::Present) {}
    DeviceSwapChain* swapChain=nullptr;
    std::vector<DeviceSemaphore*> waitSemaphores;
    uint32_t imageIndex=0;
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
struct ImageAcquireSubmit:SubmitBase
{
    ImageAcquireSubmit() : SubmitBase(SubmitType::ImageAcquire) {}
    DeviceSwapChain* swapChain=nullptr;
    DeviceSemaphore* signalSemaphore=nullptr;
    uint64_t timeoutNs=UINT64_MAX;
    ImageAcquireResult* result=nullptr;
    std::binary_semaphore* semaphore=nullptr;
};
struct CustomSubmit:SubmitBase
{
    CustomSubmit() : SubmitBase(SubmitType::Custom) {}
    std::function<void()> func;
    std::binary_semaphore* semaphore=nullptr;
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