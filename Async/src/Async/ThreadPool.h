#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include <list>
#include <assert.h>
#include <functional>
#include <optional>
#include <Core/Core.h>
#include <concurrentqueue/moodycamel/concurrentqueue.h>
namespace Aether
{
namespace AsyncDetail
{

template <typename Ret, typename Tuple>
struct GetStdFunc;
template <typename Ret, typename... Args>
struct GetStdFunc<Ret, std::tuple<Args...>>
{
    using Type = std::function<Ret(Args...)>;
};
} // namespace AsyncDetail
struct TaskBase
{
    virtual ~TaskBase() = default;
    virtual void Execute() = 0;
    virtual void OnComplete() = 0;
};
template <typename Fn>
    requires(!std::is_void_v<typename LambdaTraits<Fn>::RetType>)
struct Task : public TaskBase
{
    using FuncType = LambdaTraits<Fn>;
    using RetType = typename FuncType::RetType;
    using StdFuncType = typename AsyncDetail::GetStdFunc<RetType, typename FuncType::ArgTypes>::Type;
    using CompleteType = std::function<void(RetType&&)>;
    StdFuncType fn;
    CompleteType onComplete;
    std::optional<RetType> result;
    void Execute() override
    {
        result = fn();
    }
    void OnComplete() override
    {
        onComplete(std::move(result.value()));
    }
};
template <typename Fn>
    requires(std::is_void_v<typename LambdaTraits<Fn>::RetType>)
struct NoRetTask : public TaskBase
{
    using FuncType = LambdaTraits<Fn>;
    using RetType = typename FuncType::RetType;
    using StdFuncType = typename AsyncDetail::GetStdFunc<RetType, typename FuncType::ArgTypes>::Type;
    using CompleteType = std::function<void()>;
    StdFuncType fn;
    CompleteType onComplete;
    void Execute() override
    {
        fn();
    }
    void OnComplete() override
    {
        onComplete();
    }
};
class ThreadPool
{
public:
    explicit ThreadPool(size_t n) : m_Shutdown(false)
    {
        while (n)
        {
            m_Threads.emplace_back(Worker(*this));
            n--;
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    template <typename Fn, typename OnComplete>
    void Enqueue(Fn&& fn, OnComplete&& onComplete)
    {
        using FuncType = LambdaTraits<Fn>;
        using RetType = typename FuncType::RetType;
        Scope<TaskBase> task;
        if constexpr (std::is_void_v<RetType>)
        {
            auto t = CreateScope<NoRetTask<std::decay_t<Fn>>>();
            t->fn = std::forward<Fn>(fn);
            t->onComplete = std::forward<OnComplete>(onComplete);
            task = std::move(t);
        }
        else
        {
            auto t = CreateScope<Task<std::decay_t<Fn>>>();
            t->fn = std::forward<Fn>(fn);
            t->onComplete = std::forward<OnComplete>(onComplete);
            task = std::move(t);
        }
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Jobs.push_back(std::move(task));
        }

        m_Cond.notify_one();
    }

    void Shutdown()
    {
        // Stop all worker threads...
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Shutdown = true;
        }

        m_Cond.notify_all();

        // Join...
        for (auto& t : m_Threads)
        {
            t.join();
        }
    }
    moodycamel::ConcurrentQueue<Scope<TaskBase>>& GetCompleteQueue()
    {
        return m_CompleteQueue;
    }

private:
    struct Worker
    {
        explicit Worker(ThreadPool& pool) : m_Pool(pool)
        {
        }

        void operator()()
        {
            for (;;)
            {
                Scope<TaskBase> task;
                {
                    std::unique_lock<std::mutex> lock(m_Pool.m_Mutex);

                    m_Pool.m_Cond.wait(lock, [&] { return !m_Pool.m_Jobs.empty() || m_Pool.m_Shutdown; });

                    if (m_Pool.m_Shutdown && m_Pool.m_Jobs.empty())
                    {
                        break;
                    }

                    task = std::move(m_Pool.m_Jobs.front());
                    m_Pool.m_Jobs.pop_front();
                }

                assert(true == static_cast<bool>(task));
                task->Execute();
                m_Pool.m_CompleteQueue.enqueue(std::move(task));
            }
        }

        ThreadPool& m_Pool;
    };
    friend struct worker;

    std::vector<std::thread> m_Threads;

    std::list<Scope<TaskBase>> m_Jobs;

    bool m_Shutdown;

    std::condition_variable m_Cond;
    std::mutex m_Mutex;
    moodycamel::ConcurrentQueue<Scope<TaskBase>> m_CompleteQueue;
};
} // namespace Aether
