#pragma once
#include "TaskBase.h"
#include "Resource/VirtualResource.h"
#include <Render/RenderApi.h>
#include "RenderTask.h"
#include "Resource/ResourceAccessor.h"
namespace Aether::RenderGraph
{
class RenderGraph
{
public:
    friend class RenderTaskBuilder;

public:
    RenderGraph(Borrow<ResourceArena> arena, Borrow<ResourceLruPool> pool) :
         m_ResourceArena(arena), m_ResourceLruPool(pool)
    {
        m_ResourceAccessor = CreateScope<ResourceAccessor>(arena);
    }
    template <typename TaskDataType>
    TaskDataType AddRenderTask(const std::function<void(RenderTaskBuilder&, TaskDataType&)>& setup,
                       std::function<void(DeviceCommandBuffer&, ResourceAccessor&, TaskDataType&)>&& execute)
    {
        auto task=CreateScope<RenderTask<TaskDataType>>();
        
        task->execute = std::move(execute);
        RenderTaskBuilder builder(task, *this);
        setup(builder, task->data);
        m_Tasks.push_back(std::move(task));
        return static_cast<RenderTask<TaskDataType>*>(m_Tasks.back().get())->data;
    }
    void Compile();
    void Execute();
    void SetCommandBuffer(DeviceCommandBuffer* commandBuffer)
    {
        m_CommandBuffer = commandBuffer;
    }

private:
    DeviceCommandBuffer* m_CommandBuffer=nullptr;
    Borrow<ResourceArena> m_ResourceArena;
    Borrow<ResourceLruPool> m_ResourceLruPool;
    std::vector<Scope<TaskBase>> m_Tasks;
    std::vector<Scope<VirtualResourceBase>> m_Resources;
    Scope<ResourceAccessor> m_ResourceAccessor;
    std::unordered_map<Handle, uint32_t> m_AccessIdToResourceIndex;
};
template <typename ResourceType>
inline AccessId<ResourceType> RenderTaskBuilder::Create(const ResourceDescType<ResourceType>::Type& desc)
{
    auto id = m_Graph.m_ResourceAccessor->CreateSlot<ResourceType>(desc);
    auto resource = CreateScope<VirtualResource<ResourceType>>(desc, id);
    resource.creator = m_Task;
    m_Task->creates.push_back(resource.get());
    m_Graph.m_Resources.push_back(std::move(resource));
    m_Graph.m_AccessIdToResourceIndex[id] = static_cast<uint32_t>(m_Graph.m_Resources.size() - 1);
    return id;
}
template <typename ResourceType>
inline AccessId<ResourceType> RenderTaskBuilder::Read(AccessId<ResourceType> resourceId)
{
    auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[resourceId]];
    m_Task->reads.push_back(resource.get());
    resource->readers.push_back(m_Task);
    return resourceId;
}
template <typename ResourceType>
inline AccessId<ResourceType> RenderTaskBuilder::Write(AccessId<ResourceType> resourceId)
{
    auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[resourceId]];
    m_Task->writes.push_back(resource.get());
    resource->writers.push_back(m_Task);
    return resourceId;
}
inline RenderTaskBuilder& RenderTaskBuilder::SetRenderPassDesc(const RenderPassDesc& desc)
{
    m_Task->renderPassDesc = desc;
    auto read=[&](Handle handle)
    {
        auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[handle]];
        m_Task->reads.push_back(resource.get());
        resource->readers.push_back(m_Task.Get());
    };
    auto write=[&](Handle handle)
    {
        auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[handle]];
        m_Task->writes.push_back(resource.get());
        resource->writers.push_back(m_Task.Get());
    };
    for(size_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        read(desc.colorAttachment[i].imageView.handle);
    }
    if (desc.depthAttachment)
    {
        read(desc.depthAttachment->imageView.handle);
    }
    return *this;
}
} // namespace Aether::RenderGraph