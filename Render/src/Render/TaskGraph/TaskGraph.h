#pragma once
#include <vector>
#include <Render/RenderApi.h>
#include <Core/Core.h>
#include "Detail/TaskBuilder.h"
#include "Detail/Resource.h"
#include "Detail/Realize.h"
#include "Detail/Task.h"
namespace Aether::TaskGraph
{








class TaskGraph
{
    friend class TaskBuilder;
public:
    template <typename T>
    T* AddDeviceTask()
    {
    }
private:
    std::vector<Scope<ResourceBase>> m_Resources;
    std::vector<Scope<TaskBase>> m_Tasks;
    
};
template<typename ResourceType, typename Desc>
requires std::derived_from<ResourceType, ResourceBase>
ResourceType* TaskBuilder::Create(const std::string& tag,const Desc& desc)
{
    #if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
    m_TaskGraph->m_Resources.emplace_back(CreateScope<ResourceType>(tag,m_Task,desc));
    #else 
    m_TaskGraph->m_Resources.emplace_back(CreateScope<ResourceType>(m_Task,desc));
    #endif 
    auto* resource=m_TaskGraph->m_Resources.back().get();
    m_Task->creates.push_back(resource);
    return static_cast<ResourceType*>(resource);
}
template<typename ResourceType>
requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* TaskBuilder::Read(ResourceType* _resource)
{
    ResourceBase* resource=(ResourceBase*)_resource;
    resource->readers.push_back(m_Task);
    m_Task->reads.push_back(resource);
    return _resource;
}
template<typename ResourceType>
requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* TaskBuilder::Write(ResourceType* _resource)
{
    ResourceBase* resource=(ResourceBase*)_resource;
    resource->writers.push_back(m_Task);
    m_Task->writes.push_back(resource);
    return _resource;
}

} // namespace Aether::TaskGraph