#include <Async/ThreadPool.h>
#include <MainLoop/MainLoop.h>

#include <atomic>
#include <iostream>
#include <stdexcept>

namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void TestThreadPoolDrain()
{
    Aether::ThreadPool pool(1);
    std::atomic<int> executed = 0;
    int completed = 0;
    Check(pool.Enqueue([&] { ++executed; }, [&] { ++completed; }), "initial task was rejected");
    pool.StopAccepting();
    Check(!pool.Enqueue([] {}, [] {}), "task was accepted after StopAccepting");
    pool.JoinWorkers();
    Check(executed == 1, "accepted task did not execute before join");
    Aether::Scope<Aether::TaskBase> task;
    Check(pool.GetCompleteQueue().try_dequeue(task), "completion was lost during join");
    task->OnComplete();
    Check(completed == 1, "completion did not execute exactly once");
    pool.Shutdown();
}

void TestMainLoopExitBoundary()
{
    int begin = 0;
    int events = 0;
    int updates = 0;
    int uploads = 0;
    int renders = 0;
    float firstDelta = -1.0f;
    Aether::MainLoop::OnFrameBegin += [&] { ++begin; };
    Aether::MainLoop::OnEvent += [&] { ++events; };
    Aether::MainLoop::OnUpdate += [&](float delta) {
        ++updates;
        firstDelta = delta;
        Aether::MainLoop::Quit();
    };
    Aether::MainLoop::OnUpload += [&] { ++uploads; };
    Aether::MainLoop::OnRender += [&] { ++renders; };
    Aether::MainLoop::Run();
    Check(begin == 1 && events == 1 && updates == 1, "main loop executed an unexpected frame count");
    Check(uploads == 0 && renders == 0, "main loop produced work after Quit");
    Check(firstDelta >= 0.0f && firstDelta < 1.0f, "first frame delta was not initialized from a monotonic clock");
}
} // namespace

int main()
{
    try
    {
        TestThreadPoolDrain();
        TestMainLoopExitBoundary();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
    return 0;
}
