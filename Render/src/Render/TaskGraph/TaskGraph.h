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
    friend class RenderTaskBuilder;

public:
    template <typename TaskData>
    TaskData AddRenderTask(std::function<void(TaskBuilder&, TaskData&)> setup, std::function<void(TaskData&,DeviceCommandBuffer& commandBuffer)> execute)
    {
        m_RenderTasks.emplace_back(CreateScope<RenderTask<TaskData>>(setup, execute));
        auto* task = (RenderTask<TaskData>*)m_RenderTasks.back().get();
        RenderTaskBuilder builder(this, task);
        task->Setup(builder);
        return task->GetData();
    }
    void Compile();
    template<typename ActualType,typename DescType>
    Resource<ActualType, DescType>* AddRetainedResource(const std::string& tag,Scope<ActualType> actual,const DescType& desc)
    {
        auto r= CreateScope<Resource<ActualType, DescType>>(tag, nullptr, desc);
        r->SetActual(std::move(actual));
        m_Resources.emplace_back(std::move(r));
        return static_cast<Resource<ActualType, DescType>*>(m_Resources.back().get());
    }
    template<typename ActualType,typename DescType>
    Resource<ActualType, DescType>* AddRetainedResourceBorrow(const std::string& tag,Borrow<ActualType> actual,const DescType& desc)
    {
        auto r= CreateScope<Resource<ActualType, DescType>>(tag, nullptr, desc);
        r->SetActualBorrow(actual);
        m_Resources.emplace_back(std::move(r));
        return static_cast<Resource<ActualType, DescType>*>(m_Resources.back().get());
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
    using Task=std::variant<std::monostate,Borrow<RenderTaskBase>>;
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
        void operator()(const Borrow<RenderTaskBase>& task)
        {
            task->Execute(commandBuffer);
        }
    };
    std::vector<Scope<ResourceBase>> m_Resources;
    std::vector<Scope<RenderTaskBase>> m_RenderTasks;
    std::vector<Timeline> m_Timelines; // create on compile, execute every frame
private:// render resource
    DeviceCommandBuffer m_CommandBuffer;
    TransientResourcePool m_TransientResourcePool;
};
template <typename ResourceType, typename Desc>
    requires std::derived_from<ResourceType, ResourceBase>
ResourceType* RenderTaskBuilder::Create(const std::string& tag, const Desc& desc)
{
    m_TaskGraph->m_Resources.emplace_back(CreateScope<ResourceType>(tag, m_Task, desc));

    auto* resource = m_TaskGraph->m_Resources.back().get();
    m_Task->creates.push_back(resource);
    return static_cast<ResourceType*>(resource);
}
template <typename ResourceType>
    requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* RenderTaskBuilder::Read(ResourceType* _resource)
{
    ResourceBase* resource = (ResourceBase*)_resource;
    resource->readers.push_back(m_Task);
    m_Task->reads.push_back(resource);
    return _resource;
}
template <typename ResourceType>
    requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* RenderTaskBuilder::Write(ResourceType* _resource)
{
    ResourceBase* resource = (ResourceBase*)_resource;
    resource->writers.push_back(m_Task);
    m_Task->writes.push_back(resource);
    return _resource;
}

} // namespace Aether::TaskGraph