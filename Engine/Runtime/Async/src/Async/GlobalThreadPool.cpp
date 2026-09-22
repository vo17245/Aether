#include "GlobalThreadPool.h"
#include <algorithm>
namespace Aether
{
    void GlobalThreadPool::Init(size_t threadCount )
    {
        assert(!s_ThreadPool && "GlobalThreadPool already initialized!");
        s_ThreadPool=new ThreadPool(std::max<std::size_t>(threadCount, 1));
    }
      
    void GlobalThreadPool::Destory()
    {
        assert(s_ThreadPool && "GlobalThreadPool not initialized!");
        s_ThreadPool->StopAccepting();
        s_ThreadPool->JoinWorkers();
        delete s_ThreadPool;
        s_ThreadPool=nullptr;
    }

    void GlobalThreadPool::StopAccepting()
    {
        if (s_ThreadPool) s_ThreadPool->StopAccepting();
    }

    void GlobalThreadPool::JoinWorkers()
    {
        if (s_ThreadPool) s_ThreadPool->JoinWorkers();
    }

    bool GlobalThreadPool::IsInitialized()
    {
        return s_ThreadPool != nullptr;
    }
    ThreadPool* GlobalThreadPool::s_ThreadPool=nullptr;
}
