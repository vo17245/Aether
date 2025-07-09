#pragma once
#include <Core/Core.h>
#include "Resource.h"
namespace Aether::TaskGraph 
{
    class TaskGraph;
    class TaskBase;
    class TaskBuilder
    {
    public:
        TaskBuilder(TaskGraph* graph,TaskBase* task)
        :m_TaskGraph(graph),m_Task(task){}
        template<typename ResourceType,typename Desc>
        requires std::derived_from<ResourceType, ResourceBase>
        ResourceType* Create(const std::string& tag,const Desc& desc);

        template<typename ResourceType>
        requires std::derived_from<ResourceType, ResourceBase>
        ResourceType* Write(ResourceType* resource);
        template<typename ResourceType>
        requires std::derived_from<ResourceType, ResourceBase>
        ResourceType* Read(ResourceType* resource);
    private:
        TaskGraph* m_TaskGraph;
        TaskBase* m_Task;
    };
}