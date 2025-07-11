#include "TaskGraph.h"
#include <stack>
namespace Aether::TaskGraph
{
void TaskGraph::Compile()
{
    // Reference counting.
    for (auto& task : m_DeviceTasks)
    {
        task->refCount = task->creates.size() + task->writes.size();
    }
    for (auto& resource : m_Resources)
    {
        resource->refCount = resource->readers.size();
    }

    // Culling via flood fill from unreferenced resources.
    std::stack<ResourceBase*> unreferencedResources;
    for (auto& resource : m_Resources)
    {
        if (resource->refCount == 0 && resource->Transient())
            unreferencedResources.push(resource.get());
    }

    while (!unreferencedResources.empty())
    {
        auto unreferencedResource = unreferencedResources.top();
        unreferencedResources.pop();

        auto creator = static_cast<TaskBase*>(unreferencedResource->creator);
        if (creator->refCount > 0)
        {
            creator->refCount--;
        }
        if (creator->refCount == 0 && !creator->cullImmune)
        {
            for (auto iteratee : creator->reads)
            {
                auto read_resource = static_cast<ResourceBase*>(iteratee);
                if (read_resource->refCount > 0)
                {
                    read_resource->refCount--;
                }
                if (read_resource->refCount == 0 && read_resource->Transient())
                    unreferencedResources.push(read_resource);
            }
        }

        for (auto c_writer : unreferencedResource->writers)
        {
            auto writer = static_cast<TaskBase*>(c_writer);
            if (writer->refCount > 0)
            {
                writer->refCount--;
            }
            if (writer->refCount == 0 && !writer->cullImmune)
            {
                for (auto iteratee : writer->reads)
                {
                    auto read_resource = const_cast<ResourceBase*>(iteratee);
                    if (read_resource->refCount > 0)
                    {
                        read_resource->refCount--;
                    }
                    if (read_resource->refCount == 0 && read_resource->Transient())
                    {
                        unreferencedResources.push(read_resource);
                    }
                }
            }
        }
    }

    // Timeline computation.
    m_Timelines.clear();
    for (auto& render_task : m_DeviceTasks)
    {
        if (render_task->refCount == 0 && !render_task->cullImmune)
        {
            continue;
        }

        std::vector<Borrow<ResourceBase>> realized_resources, derealized_resources;

        for (auto resource : render_task->creates)
        {
            realized_resources.push_back(const_cast<ResourceBase*>(resource));

            if (resource->readers.empty() && resource->writers.empty())
            {
                derealized_resources.push_back(const_cast<ResourceBase*>(resource));
            }
        }

        auto reads_writes = render_task->reads;
        reads_writes.insert(reads_writes.end(), render_task->writes.begin(), render_task->writes.end());
        for (auto resource : reads_writes)
        {
            if (!resource->Transient())
            {
                continue;
            }

            auto valid = false;
            std::size_t last_index;
            if (!resource->readers.empty())
            {
                auto last_reader = std::find_if(
                    m_DeviceTasks.begin(),
                    m_DeviceTasks.end(),
                    [&resource](const std::unique_ptr<DeviceTaskBase>& iteratee) {
                        return iteratee.get() == resource->readers.back();
                    });
                if (last_reader != m_DeviceTasks.end())
                {
                    valid = true;
                    last_index = std::distance(m_DeviceTasks.begin(), last_reader);
                }
            }
            if (!resource->writers.empty())
            {
                auto last_writer = std::find_if(
                    m_DeviceTasks.begin(),
                    m_DeviceTasks.end(),
                    [&resource](const std::unique_ptr<DeviceTaskBase>& iteratee) {
                        return iteratee.get() == resource->writers.back();
                    });
                if (last_writer != m_DeviceTasks.end())
                {
                    valid = true;
                    last_index = std::max(last_index, std::size_t(std::distance(m_DeviceTasks.begin(), last_writer)));
                }
            }

            if (valid && m_DeviceTasks[last_index] == render_task)
                derealized_resources.push_back(const_cast<ResourceBase*>(resource));
        }

        m_Timelines.push_back(Timeline{render_task.get(), realized_resources, derealized_resources});
    }

}
} // namespace Aether::TaskGraph