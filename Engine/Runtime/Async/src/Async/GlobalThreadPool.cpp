#include "GlobalThreadPool.h"
namespace Aether
{
    void GlobalThreadPool::Init(size_t threadCount )
    {
        assert(!s_ThreadPool && "GlobalThreadPool already initialized!");
        s_ThreadPool=new ThreadPool(threadCount);
    }
      
    void GlobalThreadPool::Destory()
    {
        assert(s_ThreadPool && "GlobalThreadPool not initialized!");
        s_ThreadPool->Shutdown();
        delete s_ThreadPool;
        s_ThreadPool=nullptr;
    }
    ThreadPool* GlobalThreadPool::s_ThreadPool=nullptr;
}