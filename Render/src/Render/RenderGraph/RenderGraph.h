#pragma once
#include "TaskBase.h"
#include "Resource/VirtualResource.h"
#include <Render/RenderApi.h>
#include "RenderTask.h"
#include "Resource/ResourceAccessor.h"
#include "TransferTask.h"
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
        m_ResourceAccessor = CreateScope<ResourceAccessor>(arena, pool);
    }
    template <typename TaskDataType>
    TaskDataType AddRenderTask(const std::string& tag,
                               const std::function<void(RenderTaskBuilder&, TaskDataType&)>& setup,
                               std::function<void(DeviceCommandBuffer&, ResourceAccessor&, TaskDataType&)>&& execute)
    {
        auto task = CreateScope<RenderTask<TaskDataType>>();
#if AETHER_RENDER_GRAPH_ENABLE_TAG
        task->tag = tag;
#endif
        task->execute = std::move(execute);
        RenderTaskBuilder builder(static_cast<RenderTaskBase*>(task.get()), *this);
        setup(builder, task->data);
        m_Tasks.push_back(std::move(task));
        return static_cast<RenderTask<TaskDataType>*>(m_Tasks.back().get())->data;
    }

    void Compile();
    void SetCurrentFrame(uint32_t frame)
    {
        m_ResourceAccessor->SetCurrentFrame(frame);
    }
    void Execute();
    void SetCommandBuffer(DeviceCommandBuffer* commandBuffer)
    {
        m_CommandBuffer = commandBuffer;
    }
    ResourceAccessor& GetResourceAccessor()
    {
        return *m_ResourceAccessor;
    }
    template <typename ResourceType>
        requires(IsResource<ResourceType>::value)
    AccessId<ResourceType> Import(const typename ResourceDescType<ResourceType>::Type& desc,
                                  const std::span<ResourceId<ResourceType>>& ids)
    {
        auto& slot = m_ResourceAccessor->CreateSlot<ResourceType>(desc);
        for (size_t i = 0; i < ids.size(); ++i)
        {
            slot.frameResources[i] = ids[i];
            slot.isRealized[i] = true;
        }
        slot.resourceCount = static_cast<uint32_t>(ids.size());
        slot.external = true;
        auto virtualResource = CreateScope<VirtualResource<ResourceType>>(desc, slot.id);
        m_AccessIdToResourceIndex[slot.id.handle] = static_cast<uint32_t>(m_Resources.size());
        m_Resources.push_back(std::move(virtualResource));
        return slot.id;
    }

private:
    // push m_Tasks into m_Steps and cull tasks that do not affect the imported resources.
    void CullTasks();
    // Perform resource lifetime analysis and alias freed (dead) resources to reuse memory.
    void PerformResourceAliasing();
    void MergeRenderPasses();
    // insert image layout transition
    // render task: write(transfer to color/depth attachment)
    //              read(transfer to texture) tasks
    // transfer task: write(transfer to transfer dst)
    //                read(transfer to transfer src)
    void InsertImageLayoutTransition();
    // enable resource slot supports in-flight resources for mutable resources
    void SetResourceSlotSupportsInFlightResources();

private:
    DeviceRenderPassDesc RenderPassDescToDeviceRenderPassDesc(const RenderPassDesc& desc);
    FrameBufferDesc RenderPassDescToFrameBufferDesc(const RenderPassDesc& desc);

private:
    DeviceCommandBuffer* m_CommandBuffer = nullptr;
    Borrow<ResourceArena> m_ResourceArena;
    Borrow<ResourceLruPool> m_ResourceLruPool;
    std::vector<Scope<TaskBase>> m_Tasks;
    std::vector<Scope<VirtualResourceBase>> m_Resources;
    Scope<ResourceAccessor> m_ResourceAccessor;
    std::unordered_map<Handle, uint32_t, Hash<Handle>> m_AccessIdToResourceIndex;
    std::vector<TaskBase*> m_Steps; // compile result
};
template <typename ResourceType>
    requires IsResource<ResourceType>::value
AccessId<ResourceType> RenderTaskBuilder::Create(const std::string& tag,
                                                 const typename ResourceDescType<ResourceType>::Type& desc)
{
    auto& slot = m_Graph.m_ResourceAccessor->CreateSlot<ResourceType>(desc);
    auto id = slot.id;
    auto resource = CreateScope<VirtualResource<ResourceType>>(desc, id);
#if AETHER_RENDER_GRAPH_ENABLE_TAG
    resource->tag = tag;
#endif
    resource->creator = m_Task;
    m_Task->creates.push_back(resource.get());
    m_Graph.m_Resources.push_back(std::move(resource));
    m_Graph.m_AccessIdToResourceIndex[id.handle] = static_cast<uint32_t>(m_Graph.m_Resources.size() - 1);
    return id;
}
template <typename ResourceType>
    requires IsResource<ResourceType>::value
AccessId<ResourceType> RenderTaskBuilder::Read(AccessId<ResourceType> resourceId)
{
    auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[resourceId]];
    m_Task->reads.push_back(resource.get());
    resource->readers.push_back(m_Task);
    return resourceId;
}
template <typename ResourceType>
    requires IsResource<ResourceType>::value
AccessId<ResourceType> RenderTaskBuilder::Write(AccessId<ResourceType> resourceId)
{
    auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[resourceId]];
    m_Task->writes.push_back(resource.get());
    resource->writers.push_back(m_Task);
    return resourceId;
}
inline RenderTaskBuilder& RenderTaskBuilder::SetRenderPassDesc(const RenderPassDesc& desc)
{
    m_Task->renderPassDesc = desc;
    auto read = [&](Handle handle) {
        auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[handle]];
        m_Task->reads.push_back(resource.get());
        resource->readers.push_back(m_Task.Get());
    };
    auto write = [&](Handle handle) {
        auto& resource = m_Graph.m_Resources[m_Graph.m_AccessIdToResourceIndex[handle]];
        m_Task->writes.push_back(resource.get());
        resource->writers.push_back(m_Task.Get());
    };
    // set image view dependence
    for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        read(desc.colorAttachment[i].imageView.handle);
    }
    if (desc.depthAttachment)
    {
        read(desc.depthAttachment->imageView.handle);
    }
    // allocate render pass
    auto renderPassDesc = m_Graph.RenderPassDescToDeviceRenderPassDesc(desc);
    auto virtualRenderPassPtr = CreateScope<VirtualResource<DeviceRenderPass>>();
    m_Graph.m_Resources.emplace_back(std::move(virtualRenderPassPtr));
    auto& virtualRenderPass = *static_cast<VirtualResource<DeviceRenderPass>*>(m_Graph.m_Resources.back().get());
    virtualRenderPass.creator = m_Task;
    virtualRenderPass.readers.push_back(m_Task.Get());
    virtualRenderPass.desc = renderPassDesc;
    virtualRenderPass.id = m_Graph.m_ResourceAccessor->CreateSlot<DeviceRenderPass>(virtualRenderPass.desc).id;

    m_Graph.m_AccessIdToResourceIndex[virtualRenderPass.id.handle] = m_Graph.m_Resources.size() - 1;
    read(virtualRenderPass.id.handle);
    m_Task->renderPass = virtualRenderPass.id;

    // allocate frame buffer
    auto frameBufferDesc = m_Graph.RenderPassDescToFrameBufferDesc(desc);
    frameBufferDesc.renderPass = virtualRenderPass.id;
    auto virtualFrameBufferPtr = CreateScope<VirtualResource<DeviceFrameBuffer>>();
    m_Graph.m_Resources.emplace_back(std::move(virtualFrameBufferPtr));
    auto& virtualFrameBuffer = *static_cast<VirtualResource<DeviceFrameBuffer>*>(m_Graph.m_Resources.back().get());
    virtualFrameBuffer.creator = m_Task;
    virtualFrameBuffer.readers.push_back(m_Task.Get());
    virtualFrameBuffer.desc = frameBufferDesc;
    virtualFrameBuffer.id = m_Graph.m_ResourceAccessor->CreateSlot<DeviceFrameBuffer>(virtualFrameBuffer.desc).id;
    m_Graph.m_AccessIdToResourceIndex[virtualFrameBuffer.id.handle] = m_Graph.m_Resources.size() - 1;
    read(virtualFrameBuffer.id.handle);
    m_Task->frameBuffer = virtualFrameBuffer.id;

    return *this;
}
} // namespace Aether::RenderGraph