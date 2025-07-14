#pragma once
#include <Core/Core.h>
#include <Render/RenderApi.h>
using namespace Aether;
class GlobalApplicationResource
{
public:
    static GlobalApplicationResource& GetInstance()
    {
        static GlobalApplicationResource instance;
        return instance;
    }
    static void Init()
    {
        GetInstance().InitImpl();
    }
    static void Destroy()
    {
        GetInstance().m_CommandBufferPool={};
    }   
    DeviceCommandBufferPool& GetCommandBufferPool()
    {
        return m_CommandBufferPool;
    }
private:
    void InitImpl()
    {
        m_CommandBufferPool=DeviceCommandBufferPool::Create();
        assert(!m_CommandBufferPool.Empty() && "failed to create command buffer pool");
    }
    DeviceCommandBufferPool m_CommandBufferPool;
};