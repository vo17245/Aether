#include "TaskGraph.h"
#include <stack>
namespace Aether::TaskGraph
{
void TaskGraph::CalculateTimeline()
{
    // Reference counting.
    for (auto& task : m_RenderTasks)
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
    for (auto& render_task : m_RenderTasks)
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
                    m_RenderTasks.begin(),
                    m_RenderTasks.end(),
                    [&resource](const std::unique_ptr<RenderTaskBase>& iteratee) {
                        return iteratee.get() == resource->readers.back();
                    });
                if (last_reader != m_RenderTasks.end())
                {
                    valid = true;
                    last_index = std::distance(m_RenderTasks.begin(), last_reader);
                }
            }
            if (!resource->writers.empty())
            {
                auto last_writer = std::find_if(
                    m_RenderTasks.begin(),
                    m_RenderTasks.end(),
                    [&resource](const std::unique_ptr<RenderTaskBase>& iteratee) {
                        return iteratee.get() == resource->writers.back();
                    });
                if (last_writer != m_RenderTasks.end())
                {
                    valid = true;
                    last_index = std::max(last_index, std::size_t(std::distance(m_RenderTasks.begin(), last_writer)));
                }
            }

            if (valid && m_RenderTasks[last_index] == render_task)
                derealized_resources.push_back(const_cast<ResourceBase*>(resource));
        }

        m_Timelines.push_back(Timeline{render_task.get(), realized_resources, derealized_resources});
    }
}
void TaskGraph::MergeRenderPass()
{
    std::optional<RenderPassDesc> desc;
    RenderTaskArray currentArray;
    std::vector<Timeline> newTimelines;
    std::vector<Borrow<ResourceBase>> realizedResources;   // should realize before task array
    std::vector<Borrow<ResourceBase>> derealizedResources; // should derealize after task array
    std::vector<LayoutTransition> layoutTransitions;       // should transition layout before task array(execute after resources realize)
    for (auto& timeline : m_Timelines)
    {
        auto& task = timeline.task;
        if (std::holds_alternative<Borrow<RenderTaskBase>>(task))
        {
            auto renderTask = std::get<Borrow<RenderTaskBase>>(task);
            if (desc)
            {
                if (*desc == renderTask->GetRenderPassDesc())
                {
                    // add to current array
                    currentArray.tasks.push_back(renderTask);
                    realizedResources.insert(realizedResources.end(),
                                             timeline.realizedResources.begin(),
                                             timeline.realizedResources.end());
                    derealizedResources.insert(derealizedResources.end(),
                                               timeline.derealizedResources.begin(),
                                               timeline.derealizedResources.end());
                    layoutTransitions.insert(layoutTransitions.end(),
                                             timeline.layoutTransitions.begin(),
                                             timeline.layoutTransitions.end());
                }
                else
                {
                    // finalize current array
                    currentArray.renderPassDesc = *desc;
                    newTimelines.push_back(Timeline{.task = currentArray,
                                                    .realizedResources = realizedResources,
                                                    .derealizedResources = derealizedResources,
                                                    .layoutTransitions = layoutTransitions});

                    // reset for next array
                    currentArray.tasks.clear();
                    realizedResources.clear();
                    derealizedResources.clear();
                    layoutTransitions.clear();
                    // add current render task to new array
                    desc = renderTask->GetRenderPassDesc();
                    currentArray.tasks.push_back(renderTask);
                    realizedResources.insert(realizedResources.end(),
                                             timeline.realizedResources.begin(),
                                             timeline.realizedResources.end());
                    derealizedResources.insert(derealizedResources.end(),
                                               timeline.derealizedResources.begin(),
                                               timeline.derealizedResources.end());
                    layoutTransitions.insert(layoutTransitions.end(),
                                             timeline.layoutTransitions.begin(),
                                             timeline.layoutTransitions.end());
                }
            }
            else
            {
                // start new array
                desc = renderTask->GetRenderPassDesc();
                currentArray.tasks.push_back(renderTask);
                realizedResources.insert(realizedResources.end(),
                                         timeline.realizedResources.begin(),
                                         timeline.realizedResources.end());
                derealizedResources.insert(derealizedResources.end(),
                                           timeline.derealizedResources.begin(),
                                           timeline.derealizedResources.end());
                layoutTransitions.insert(layoutTransitions.end(),
                                         timeline.layoutTransitions.begin(),
                                         timeline.layoutTransitions.end());
            }
        }
        else
        {
            if (desc)
            {
                // finalize current array
                currentArray.renderPassDesc = *desc;
                newTimelines.push_back(Timeline{
                    .task = currentArray,
                    .realizedResources = realizedResources,
                    .derealizedResources = derealizedResources,
                });
                // reset for next array
                currentArray.tasks.clear();
                realizedResources.clear();
                derealizedResources.clear();
                desc.reset();
            }
            // add non render task to new timelines
            newTimelines.push_back(std::move(timeline));
        }
    }
}
void TaskGraph::CreateTextureLayoutTransitions()
{
    for (auto& timeline : m_Timelines)
    {
        auto& task = timeline.task;
        if (std::holds_alternative<Borrow<RenderTaskBase>>(task))
        {
            auto renderTask = std::get<Borrow<RenderTaskBase>>(task);
            for (auto& resource : renderTask->reads)
            {
                if (resource->type == ResourceType::Texture)
                {
                    auto texture = static_cast<Texture*>(resource);
                    auto& desc = texture->GetDesc();
                    if (desc.layout != DeviceImageLayout::Texture)
                    {
                        timeline.layoutTransitions.push_back(LayoutTransition{

                            .oldLayout = desc.layout,
                            .newLayout = DeviceImageLayout::Texture,

                            .texture = texture,
                        });
                    }
                    desc.layout = DeviceImageLayout::Texture; // update layout to texture
                }
            }
            {
                auto& passDesc=renderTask->GetRenderPassDesc();
                for(size_t i=0;i<passDesc.colorAttachmentCount;++i)
                {
                    auto& colorAttachment = passDesc.colorAttachment[i];
                    if (colorAttachment.texture->GetDesc().layout != DeviceImageLayout::ColorAttachment)
                    {
                        timeline.layoutTransitions.push_back(LayoutTransition{
                            .oldLayout = colorAttachment.texture->GetDesc().layout,
                            .newLayout = DeviceImageLayout::ColorAttachment,
                            .texture = colorAttachment.texture,
                        });
                    }
                    colorAttachment.texture->GetDesc().layout = DeviceImageLayout::ColorAttachment; // update layout to color attachment
                }
                if(passDesc.depthAttachment)
                {
                    if (passDesc.depthAttachment->texture->GetDesc().layout != DeviceImageLayout::DepthStencilAttachment)
                    {
                        timeline.layoutTransitions.push_back(LayoutTransition{
                            .oldLayout = passDesc.depthAttachment->texture->GetDesc().layout,
                            .newLayout = DeviceImageLayout::DepthStencilAttachment,
                            .texture = passDesc.depthAttachment->texture,
                        });
                    }
                    passDesc.depthAttachment->texture->GetDesc().layout = DeviceImageLayout::DepthStencilAttachment; // update layout to depth stencil attachment
                }
            }
        }
    }
}
} // namespace Aether::TaskGraph