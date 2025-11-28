#pragma once
#include <Core/Core.h>
#include <Render/RenderApi.h>
namespace Aether::Render
{

struct Submit
{
    DeviceCommandBuffer* commandBuffer=nullptr;
    DeviceFence* signalFence=nullptr;
    std::vector<DevicePipelineSyncStage> waitStages;
    std::vector<DeviceSemaphore*> waitSemaphores;
    std::vector<DeviceSemaphore*> signalSemaphores;
};
struct PresentSubmit
{

};
class SubmitThread
{
public:
    static void Init();
    static void Shutdown();
    template<typename T>
        requires std::is_convertible_v<T, Submit>
    static void PushGraphics(T&& commandBuffer)
    {
        assert(GetSingleton().m_Dispatcher);
        GetSingleton().m_Dispatcher->PushGraphics(std::forward<T>(commandBuffer));
        GetSingleton().m_Condition.notify_one();
    }
    template<typename T>
        requires std::is_convertible_v<T, Submit>
    static void PushTransfer(T&& commandBuffer)
    {
        assert(GetSingleton().m_Dispatcher);
        GetSingleton().m_Dispatcher->PushTransfer(std::forward<T>(commandBuffer));
        GetSingleton().m_Condition.notify_one();
    }
    template<typename T>
        requires std::is_convertible_v<T, Submit>
    static void PushCompute(T&& commandBuffer)
    {
        assert(GetSingleton().m_Dispatcher);
        GetSingleton().m_Dispatcher->PushCompute(std::forward<T>(commandBuffer));
        GetSingleton().m_Condition.notify_one();
    }
private:
    static SubmitThread& GetSingleton();
    void InitImpl();
    void ShutdownImpl();
    struct Worker
    {
        SubmitThread& thread;
        LockFree::Queue<Submit>& queue;
        void operator()();

    };

    std::optional<std::thread> m_Thread;
    std::atomic<bool> m_Running = false;
    std::mutex m_Mutex;
    std::condition_variable m_Condition;

private:
    struct Dispatcher
    {
        LockFree::Queue<Submit>& graphicsQueue;
        LockFree::Queue<Submit>& transferQueue;
        LockFree::Queue<Submit>& computeQueue;
        template<typename T>
        requires std::is_convertible_v<T, Submit>
        void PushGraphics(T&& commandBuffer)
        {
            graphicsQueue.enqueue(std::forward<T>(commandBuffer));
        }
        template<typename T>
        requires std::is_convertible_v<T, Submit>
        void PushTransfer(T&& commandBuffer)
        {
            transferQueue.enqueue(std::forward<T>(commandBuffer));
        }
        template<typename T>
        requires std::is_convertible_v<T, Submit>
        void PushCompute(T&& commandBuffer)
        {
            computeQueue.enqueue(std::forward<T>(commandBuffer));
        }
    };
    LockFree::Queue<Submit> m_CommandBufferQueue;
    std::optional<Dispatcher> m_Dispatcher;
};
} // namespace Aether::Render