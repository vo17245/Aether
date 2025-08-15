#include "RenderGraph.h"
#include <stack>
#include "ImageLayoutTransitionTask.h"

namespace Aether::RenderGraph
{
void RenderGraph::Compile()
{
    CullTasks();
    PerformResourceAliasing();
    MergeRenderPasses();
    InsertImageLayoutTransition();
    SetResourceSlotSupportsInFlightResources();
}
void RenderGraph::CullTasks()
{
    // Reference counting.
    for (auto& task : m_Tasks)
    {
        task->refCount = task->creates.size() + task->writes.size();
    }
    for (auto& resource : m_Resources)
    {
        resource->refCount = resource->readers.size();
    }

    // Culling via flood fill from unreferenced resources.
    std::stack<VirtualResourceBase*> unreferencedResources;
    for (auto& resource : m_Resources)
    {
        if (resource->refCount == 0 && resource->creator == nullptr)
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
                auto read_resource = static_cast<VirtualResourceBase*>(iteratee);
                if (read_resource->refCount > 0)
                {
                    read_resource->refCount--;
                }
                if (read_resource->refCount == 0 && read_resource->creator == nullptr)
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
                    auto read_resource = static_cast<VirtualResourceBase*>(iteratee);
                    if (read_resource->refCount > 0)
                    {
                        read_resource->refCount--;
                    }
                    if (read_resource->refCount == 0 && read_resource->creator == nullptr)
                    {
                        unreferencedResources.push(read_resource);
                    }
                }
            }
        }
    }

    // Timeline
    m_Steps.clear();
    for (auto& render_task : m_Tasks)
    {
        if (render_task->refCount == 0 && !render_task->cullImmune)
        {
            continue;
        }
        m_Steps.push_back(render_task.get());
    }
}
namespace Detail
{
template <typename... Ts>
class VirtualResourcePoolBase
{
public:
    template <typename T>
    void Push(VirtualResource<T>* resource)
    {
        auto& resourceMap = std::get<ResourceMap<T>>(m_ResourceMaps);
        auto iter = resourceMap.find(resource->desc);
        if (iter == resourceMap.end())
        {
            iter = resourceMap.emplace(resource->desc, std::vector<VirtualResource<T>*>()).first;
        }
        iter->second.push_back(resource);
    }
    template <typename T>
    VirtualResource<T>* Pop(const typename ResourceDescType<T>::Type& desc)
    {
        auto& resourceMap = std::get<ResourceMap<T>>(m_ResourceMaps);
        auto iter = resourceMap.find(desc);
        if (iter != resourceMap.end() && !iter->second.empty())
        {
            VirtualResource<T>* resource = iter->second.back();
            iter->second.pop_back();
            return resource;
        }
        return nullptr;
    }

private:
    template <typename T>
    using ResourceMap = std::unordered_map<typename ResourceDescType<T>::Type, std::vector<VirtualResource<T>*>,
                                           Hash<typename ResourceDescType<T>::Type>>;
    std::tuple<ResourceMap<Ts>...> m_ResourceMaps;
};
template <typename T>
struct BuildVirtualResourcePool;
template<typename... Ts>
struct BuildVirtualResourcePool<TypeArray<Ts...>>
{
    using Type = VirtualResourcePoolBase<Ts...>;
};
using VirtualResourcePool = typename BuildVirtualResourcePool<ResourceTypeArray>::Type;
} // namespace Detail
void RenderGraph::PerformResourceAliasing()
{
    // caculate last access task index in m_Steps for each resource
    for (auto& resource : m_Resources)
    {
        auto lastReader = std::find_if(m_Steps.begin(), m_Steps.end(),
                                       [&](const TaskBase* task) { return task == resource->readers.back().Get(); });
        assert(lastReader != m_Steps.end());
        resource->lastAccessTaskIndex = std::distance(m_Steps.begin(), lastReader);
        auto lastWriter = std::find_if(m_Steps.begin(), m_Steps.end(),
                                       [&](const TaskBase* task) { return task == resource->writers.back().Get(); });
        assert(lastWriter != m_Steps.end());
        resource->lastAccessTaskIndex =
            std::max(resource->lastAccessTaskIndex, size_t(std::distance(m_Steps.begin(), lastWriter)));
    }
    // perform resource aliasing
    auto pool = Detail::VirtualResourcePool();
    for (size_t i = 0; i < m_Steps.size(); ++i)
    {
        auto& task = m_Steps[i];
        // try to find a compatible resource in the pool
        for (auto& resource : task->creates)
        {
            auto* compatibleResource = VisitVirtualResource(*resource, [&](auto&& r)->VirtualResourceBase*{
                
                return pool.Pop<typename std::decay_t<decltype(r)>::ResourceType>(r.desc);
            });
            if(compatibleResource)
            {
                // forward the resource id
                VisitVirtualResource(*resource, [&](auto&& r)->void{
                    using ResourceType= typename std::decay_t<decltype(r)>::ResourceType;
                    VirtualResource<ResourceType>& from= static_cast<VirtualResource<ResourceType>&>(*compatibleResource);
                    m_ResourceAccessor->Forward(from.id,r.id);
                });
            }
        }
        // push dead resources to the pool
        for(auto& resource: task->reads)
        {
            if(resource->lastAccessTaskIndex==i)
            {
                VisitVirtualResource(*resource, [&](auto&& r)->void{
                    using ResourceType= typename std::decay_t<decltype(r)>::ResourceType;
                    pool.Push(new VirtualResource<ResourceType>(r.desc,r.id));
                });
            }
        }
        for(auto& resource: task->writes)
        {
            if(resource->lastAccessTaskIndex==i)
            {
                VisitVirtualResource(*resource, [&](auto&& r)->void{
                    using ResourceType= typename std::decay_t<decltype(r)>::ResourceType;
                    pool.Push(new VirtualResource<ResourceType>(r.desc,r.id));
                });
            }
        }
    }
}
void RenderGraph::InsertImageLayoutTransition()
{
    
    std::vector<TaskBase*> newSteps;
    for(auto* _task:m_Steps)
    {
        auto& task = *_task;
        // insert image layout transition tasks
        if(task.type==TaskType::RenderTask)
        {
            auto& renderTask = static_cast<RenderTaskBase&>(task);
            // handle texture sampling
            for(auto& resource:task.reads)
            {
                if(resource->code==ResourceCode::Texture)
                {
                    auto& texture = static_cast<VirtualResource<DeviceTexture>&>(*resource);
                    AccessId<DeviceTexture> id=texture.id;
                    auto& slot=m_ResourceAccessor->GetSlot(id);
                    if(slot.virtualInfo.layout!=DeviceImageLayout::Texture)
                    {
                        auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                        transitionTask->texture = id;
                        transitionTask->oldLayout = slot.virtualInfo.layout;
                        transitionTask->newLayout = DeviceImageLayout::Texture;
                        m_Tasks.push_back(std::move(transitionTask));
                        newSteps.push_back(m_Tasks.back().get());
                        slot.virtualInfo.layout = DeviceImageLayout::Texture;
                    }
                }
            }
            // handle color attachment
            for(size_t i=0;i<renderTask.renderPassDesc.colorAttachmentCount;i++)
            {
                auto& colorAttachment = renderTask.renderPassDesc.colorAttachment[i];
                auto& viewSlot= m_ResourceAccessor->GetSlot(colorAttachment.imageView);
                auto& texture=m_ResourceAccessor->GetSlot(viewSlot.desc.texture);
                if(texture.virtualInfo.layout!=DeviceImageLayout::ColorAttachment)
                {
                    auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                    transitionTask->texture = texture.id;
                    transitionTask->oldLayout = texture.virtualInfo.layout;
                    transitionTask->newLayout = DeviceImageLayout::ColorAttachment;
                    m_Tasks.push_back(std::move(transitionTask));
                    newSteps.push_back(m_Tasks.back().get());
                    texture.virtualInfo.layout = DeviceImageLayout::ColorAttachment;
                }
            }
            // handle depth attachment
            if(renderTask.renderPassDesc.depthAttachment)
            {
                auto& depthAttachment = *renderTask.renderPassDesc.depthAttachment;
                auto& viewSlot = m_ResourceAccessor->GetSlot(depthAttachment.imageView);
                auto& texture = m_ResourceAccessor->GetSlot(viewSlot.desc.texture);
                if(texture.virtualInfo.layout!=DeviceImageLayout::DepthStencilAttachment)
                {
                    auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                    transitionTask->texture = texture.id;
                    transitionTask->oldLayout = texture.virtualInfo.layout;
                    transitionTask->newLayout = DeviceImageLayout::DepthStencilAttachment;
                    m_Tasks.push_back(std::move(transitionTask));
                    newSteps.push_back(m_Tasks.back().get());
                    texture.virtualInfo.layout = DeviceImageLayout::DepthStencilAttachment;
                }
            }
        }
        else if(task.type==TaskType::UploadTextureTask)
        {
            auto& uploadTask = static_cast<UploadTextureTask&>(task);
            auto& sourceSlot = m_ResourceAccessor->GetSlot(uploadTask.source);
            auto& destinationSlot = m_ResourceAccessor->GetSlot(uploadTask.destination);
            if(destinationSlot.virtualInfo.layout != DeviceImageLayout::TransferDst)
            {
                auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                transitionTask->texture = uploadTask.destination;
                transitionTask->oldLayout = destinationSlot.virtualInfo.layout;
                transitionTask->newLayout = DeviceImageLayout::TransferDst;
                m_Tasks.push_back(std::move(transitionTask));
                newSteps.push_back(m_Tasks.back().get());
                destinationSlot.virtualInfo.layout = DeviceImageLayout::TransferDst;
            }
        }
        else if(task.type==TaskType::DownloadTextureTask)
        {
            auto& downloadTask = static_cast<DownloadTextureTask&>(task);
            auto& sourceSlot = m_ResourceAccessor->GetSlot(downloadTask.source);
            auto& destinationSlot = m_ResourceAccessor->GetSlot(downloadTask.destination);
            if(sourceSlot.virtualInfo.layout != DeviceImageLayout::TransferSrc)
            {
                auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                transitionTask->texture = downloadTask.source;
                transitionTask->oldLayout = sourceSlot.virtualInfo.layout;
                transitionTask->newLayout = DeviceImageLayout::TransferSrc;
                m_Tasks.push_back(std::move(transitionTask));
                newSteps.push_back(m_Tasks.back().get());
                sourceSlot.virtualInfo.layout = DeviceImageLayout::TransferSrc;
            }
        }
        // add the task to new steps
        newSteps.push_back(&task);
    }
}
void RenderGraph::MergeRenderPasses()
{
    bool inPass=false;
    RenderPassDesc currentPassDesc;
    RenderTaskBase* lastRenderTask=nullptr;
    for(auto* _task:m_Steps)
    {
        auto& task = *_task;
        if(inPass)
        {
            if(task.type==TaskType::RenderTask)
            {
                auto& renderTask = static_cast<RenderTaskBase&>(task);
                if(renderTask.renderPassDesc!=currentPassDesc)
                {
                    // 结束上一个render pass
                    lastRenderTask->skipRenderPassEnd=false;
                    // 开始新的render pass
                    currentPassDesc=renderTask.renderPassDesc;
                    lastRenderTask=&renderTask;
                    renderTask.skipRenderPassEnd=true;
                }
                else
                {
                    // 合并render pass
                    renderTask.skipRenderPassBegin=true;
                    renderTask.skipRenderPassEnd=true;
                    lastRenderTask=&renderTask;
                }
            }
            else
            {
                // 结束上一个render pass
                lastRenderTask->skipRenderPassEnd=false;
                inPass=false;
            }
        }
        else
        {
            if(task.type==TaskType::RenderTask)
            {
                // 开始新的render pass
                auto& renderTask = static_cast<RenderTaskBase&>(task);
                inPass=true;
                currentPassDesc=renderTask.renderPassDesc;
                lastRenderTask=&renderTask;
                renderTask.skipRenderPassEnd=true;
            }
        }
    }

}
void RenderGraph::SetResourceSlotSupportsInFlightResources()
{
    for(auto& resource: m_Resources)
    {
        if(resource->code==ResourceCode::ImageView)
        {

        }
        else if(resource->code==ResourceCode::FrameBuffer)
        {

        }
        else
        {
            if(!resource->writers.empty())
            {
                VisitVirtualResource(*resource, [&](auto&& r)->void{
                    m_ResourceAccessor->SetSlotSupportsInFlightResources(r.id);
                });
            }
        }
    }
}
} // namespace Aether::RenderGraph