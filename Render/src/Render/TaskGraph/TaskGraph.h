#pragma once
#include <vector>
#include <Render/RenderApi.h>
#include <Core/Core.h>
#include "TaskBuilder.h"
#include "Resource.h"
#include "Realize.h"
#include "Task.h"
#include <variant>
#include "TransientResourcePool.h"
namespace Aether::TaskGraph
{

class TaskGraph
{
    friend class TaskBuilder;

public:
    template <typename TaskData>
    TaskData AddDeviceTask(std::function<void(TaskBuilder&, TaskData&)> setup, std::function<void(TaskData&)> execute)
    {
        m_DeviceTasks.emplace_back(CreateScope<DeviceTask<TaskData>>(setup, execute));
        auto* task = (DeviceTask<TaskData>*)m_DeviceTasks.back().get();
        TaskBuilder builder(this, task);
        task->Setup(builder);
        return task->GetData();
    }
    void Compile();
    void AddRetainedResource(Scope<ResourceBase>&&  resource)
    {
        m_Resources.emplace_back(std::move(resource));
    }
    bool CheckGraph()
    {
        // TODO: check if cycle exists
        return true;
    }
    TaskGraph(DeviceCommandBuffer&& commandBuffer)
        : m_CommandBuffer(std::move(commandBuffer))
    {
    }
    void Execute()
    {
        for(auto& timeline:m_Timelines)
        {
            for(auto& resource:timeline.realizedResources)
            {
                RealizeResource(*resource);
            }
            ExecuteTask executeTask{m_CommandBuffer};
            std::visit(executeTask, timeline.task);
            for(auto& resource:timeline.derealizedResources)
            {
                DerealizeResource(*resource);
            }
        }
    }
private:
    void RealizeResource(ResourceBase& resource)
    {
        switch (resource.type) 
        {
            case ResourceType::FrameBuffer:
            {
                FrameBuffer& fbr=static_cast<FrameBuffer&>(resource);
                auto fb=m_TransientResourcePool.GetFrameBuffer(fbr.GetDesc());
                if(fb)
                {
                    fbr.SetActual(std::move(fb));
                }
                else 
                {
                    fbr.Realize();
                }
            }
            break;
            default:
            resource.Realize();
        }
    }
    void DerealizeResource(ResourceBase& resource)
    {
        switch (resource.type) 
        {
            case ResourceType::FrameBuffer:
            {
                FrameBuffer& fbr=static_cast<FrameBuffer&>(resource);
                m_TransientResourcePool.PushFrameBufferf(std::move(fbr.GetActual()), fbr.GetDesc());
            }
            break;
            default:
            resource.Derealize();
        }
    }
    using Task=std::variant<std::monostate,Borrow<DeviceTaskBase>>;
    struct Timeline
    {
        Task task;
        std::vector<Borrow<ResourceBase>> realizedResources;   // should realize before task
        std::vector<Borrow<ResourceBase>> derealizedResources; // should derealize after task
    };
    struct ExecuteTask
    {
        DeviceCommandBuffer& commandBuffer;
        void operator()(const std::monostate&)
        {
            return;
        }
        void operator()(const Borrow<DeviceTaskBase>& task)
        {
            task->Execute(commandBuffer);
        }
    };
    std::vector<Scope<ResourceBase>> m_Resources;
    std::vector<Scope<DeviceTaskBase>> m_DeviceTasks;
    std::vector<Timeline> m_Timelines; // create on compile, execute every frame
private:// render resource
    DeviceCommandBuffer m_CommandBuffer;
    TransientResourcePool m_TransientResourcePool;
};
template <typename ResourceType, typename Desc>
    requires std::derived_from<ResourceType, ResourceBase>
ResourceType* TaskBuilder::Create(const std::string& tag, const Desc& desc)
{
    m_TaskGraph->m_Resources.emplace_back(CreateScope<ResourceType>(tag, m_Task, desc));

    auto* resource = m_TaskGraph->m_Resources.back().get();
    m_Task->creates.push_back(resource);
    return static_cast<ResourceType*>(resource);
}
template <typename ResourceType>
    requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* TaskBuilder::Read(ResourceType* _resource)
{
    ResourceBase* resource = (ResourceBase*)_resource;
    resource->readers.push_back(m_Task);
    m_Task->reads.push_back(resource);
    return _resource;
}
template <typename ResourceType>
    requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* TaskBuilder::Write(ResourceType* _resource)
{
    ResourceBase* resource = (ResourceBase*)_resource;
    resource->writers.push_back(m_Task);
    m_Task->writes.push_back(resource);
    return _resource;
}

} // namespace Aether::TaskGraph