#pragma once
#include <Core/Core.h>
#include "Resource.h"
namespace Aether::TaskGraph 
{
    class TaskGraph;
    class RenderTaskBase;
    class RenderTaskBuilder
    {
    public:
        RenderTaskBuilder(TaskGraph* graph,RenderTaskBase* task)
        :m_TaskGraph(graph),m_Task(task){}
        template<typename ResourceType,typename Desc>
        requires std::derived_from<ResourceType, ResourceBase>
        ResourceId<ResourceType> Create(const std::string& tag,const Desc& desc);

        template<typename ResourceType>
        requires std::derived_from<ResourceType, ResourceBase>
        ResourceId<ResourceType> Write(ResourceId<ResourceType> resource);
        template<typename ResourceType>
        requires std::derived_from<ResourceType, ResourceBase>
        ResourceId<ResourceType> Read(ResourceId<ResourceType> resource);
        
    private:
        TaskGraph* m_TaskGraph;
        RenderTaskBase* m_Task;
    };
}