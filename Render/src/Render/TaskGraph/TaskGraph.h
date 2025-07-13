#pragma once
#include <vector>
#include <Render/RenderApi.h>
#include <Core/Core.h>
#include "Detail/TaskBuilder.h"
#include "Detail/Resource.h"
#include "Detail/Realize.h"
#include "Detail/Task.h"
#include <variant>
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
private:
    struct Timeline
    {
        std::variant<std::monostate,Borrow<DeviceTaskBase>> task;
        std::vector<Borrow<ResourceBase>> realizedResources;   // should realize before task
        std::vector<Borrow<ResourceBase>> derealizedResources; // should derealize after task
    };
    std::vector<Scope<ResourceBase>> m_Resources;
    std::vector<Scope<DeviceTaskBase>> m_DeviceTasks;
    std::vector<Timeline> m_Timelines; // create on compile, execute every frame
private:// render resource
    DeviceCommandBuffer m_CommandBuffer;
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