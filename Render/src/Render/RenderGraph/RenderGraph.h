#pragma once
#include "TaskBase.h"
#include "VirtualResource.h"
#include <Render/RenderApi.h>
#include "RenderTask.h"
#include "ResourceAccessor.h"
namespace Aether::RenderGraph
{
class RenderGraph
{
public:
    friend class RenderTaskBuilder;
public:
    RenderGraph(Borrow<ResourceArena> arena,Borrow<ResourceLruPool> pool,Borrow<DeviceCommandBuffer> commandBuffer)
        : m_CommandBuffer(commandBuffer), m_ResourceArena(arena), m_ResourceLruPool(pool)
    {
        m_ResourceAccessor = CreateScope<ResourceAccessor>(arena);
    }
private:
    Borrow<DeviceCommandBuffer> m_CommandBuffer;
    Borrow<ResourceArena> m_ResourceArena;
    Borrow<ResourceLruPool> m_ResourceLruPool;
    std::vector<Scope<TaskBase>> m_Tasks;
    std::vector<Scope<VirtualResourceBase>> m_Resources;
    Scope<ResourceAccessor> m_ResourceAccessor;
};
template<typename ResourceType>
AccessId<ResourceType> RenderTaskBuilder::Create(const ResourceDescType<ResourceType>::Type& desc)
{
   
}
}