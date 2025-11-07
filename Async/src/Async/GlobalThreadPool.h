#pragma once
#include <Async/ThreadPool.h>

namespace Aether
{
class GlobalThreadPool
{
public:
    static void Init(size_t threadCount );
    template <typename Fn, typename OnComplete>
    static void Enqueue(Fn&& fn, OnComplete&& onComplete)
    {
        assert(s_ThreadPool && "GlobalThreadPool not initialized!");
        s_ThreadPool->Enqueue(std::forward<Fn>(fn), std::forward<OnComplete>(onComplete));
    }
    static void Destory();
    static moodycamel::ConcurrentQueue<Scope<TaskBase>>& GetCompleteQueue()
    {
        assert(s_ThreadPool && "GlobalThreadPool not initialized!");
        return s_ThreadPool->GetCompleteQueue();
    }

private:
    static ThreadPool* s_ThreadPool;
};
} // namespace Aether