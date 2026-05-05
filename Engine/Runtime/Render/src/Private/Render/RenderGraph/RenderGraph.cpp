#include "Render/RenderGraph/RenderGraph.h"
#include <stack>
#include "Render/RenderGraph/ImageLayoutTransitionTask.h"

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
                auto read_resource = static_cast<VirtualResourceBase*>(iteratee);
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
                    auto read_resource = static_cast<VirtualResourceBase*>(iteratee);
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
template <typename... Ts>
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
        if (!resource->readers.empty())
        {
            auto lastReader = std::find_if(m_Steps.begin(), m_Steps.end(), [&](const TaskBase* task) {
                return task == resource->readers.back().Get();
            });
            assert(lastReader != m_Steps.end());
            resource->lastAccessTaskIndex = std::distance(m_Steps.begin(), lastReader);
        }
        if (!resource->writers.empty())
        {
            auto lastWriter = std::find_if(m_Steps.begin(), m_Steps.end(), [&](const TaskBase* task) {
                return task == resource->writers.back().Get();
            });
            assert(lastWriter != m_Steps.end());
            resource->lastAccessTaskIndex =
                std::max(resource->lastAccessTaskIndex, int64_t(std::distance(m_Steps.begin(), lastWriter)));
        }
    }
    // perform resource aliasing
    auto pool = Detail::VirtualResourcePool();
    for (size_t i = 0; i < m_Steps.size(); ++i)
    {
        auto& task = m_Steps[i];
        // try to find a compatible resource in the pool
        for (auto& resource : task->creates)
        {
            auto* compatibleResource = VisitVirtualResource(*resource, [&](auto&& r) -> VirtualResourceBase* {
                return pool.Pop<typename std::decay_t<decltype(r)>::ResourceType>(r.desc);
            });
            if (compatibleResource)
            {
                // forward the resource id
                VisitVirtualResource(*resource, [&](auto&& r) -> void {
                    using ResourceType = typename std::decay_t<decltype(r)>::ResourceType;
                    VirtualResource<ResourceType>& from =
                        static_cast<VirtualResource<ResourceType>&>(*compatibleResource);
                    m_ResourceAccessor->Forward(from.id, r.id);
                });
            }
        }
        // push dead resources to the pool
        for (auto& resource : task->reads)
        {
            if (resource->lastAccessTaskIndex <= (int64_t)i)
            {
                VisitVirtualResource(*resource, [&](auto&& r) -> void {
                    using ResourceType = typename std::decay_t<decltype(r)>::ResourceType;
                    pool.Push(new VirtualResource<ResourceType>(r.desc, r.id));
                });
            }
        }
        for (auto& resource : task->writes)
        {
            if (resource->lastAccessTaskIndex <= (int64_t)i)
            {
                VisitVirtualResource(*resource, [&](auto&& r) -> void {
                    using ResourceType = typename std::decay_t<decltype(r)>::ResourceType;
                    pool.Push(new VirtualResource<ResourceType>(r.desc, r.id));
                });
            }
        }
    }
}
void RenderGraph::InsertImageLayoutTransition()
{
    std::vector<TaskBase*> newSteps;
    for (auto* _task : m_Steps)
    {
        auto& task = *_task;
        // insert image layout transition tasks
        if (task.type == TaskType::RenderTask)
        {
            auto& renderTask = static_cast<RenderTaskBase&>(task);
            // handle texture sampling
            for (auto& resource : task.reads)
            {
                if (resource->code == ResourceCode::Texture)
                {
                    auto& texture = static_cast<VirtualResource<rhi::Texture2D>&>(*resource);
                    AccessId<rhi::Texture2D> id = texture.id;
                    auto& slot = m_ResourceAccessor->GetSlot(id);
                    if (slot.virtualInfo.layout != rhi::TextureLayout::ShaderReadOnly)
                    {
                        auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                        transitionTask->texture = id;
                        transitionTask->oldLayout = slot.virtualInfo.layout;
                        transitionTask->newLayout = rhi::TextureLayout::ShaderReadOnly;
                        transitionTask->tag="Transition"+CreateUniqueId();
                        transitionTask->writes.push_back(texture);
                        m_Tasks.push_back(std::move(transitionTask));
                        newSteps.push_back(m_Tasks.back().get());
                        slot.virtualInfo.layout = rhi::TextureLayout::ShaderReadOnly;
                    }
                }
            }
            // handle color attachment
            for (size_t i = 0; i < renderTask.renderPassDesc.colorAttachmentCount; i++)
            {
                auto& colorAttachment = renderTask.renderPassDesc.colorAttachment[i];
                auto& viewSlot = m_ResourceAccessor->GetSlot(colorAttachment.textureView);
                auto* virtualResource=m_Resources[m_AccessIdToResourceIndex[viewSlot.desc.texture.handle]].get();
                auto& texture = m_ResourceAccessor->GetSlot(viewSlot.desc.texture);
                if (texture.virtualInfo.layout != rhi::TextureLayout::ColorAttachment)
                {
                    auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                    transitionTask->texture = texture.id;
                    transitionTask->oldLayout = texture.virtualInfo.layout;
                    transitionTask->newLayout = rhi::TextureLayout::ColorAttachment;
                    transitionTask->writes.push_back(virtualResource);
                    transitionTask->tag="Transition"+CreateUniqueId();
                    m_Tasks.push_back(std::move(transitionTask));
                    newSteps.push_back(m_Tasks.back().get());
                    texture.virtualInfo.layout = rhi::TextureLayout::ColorAttachment;
                }
            }
            // handle depth attachment
            if (renderTask.renderPassDesc.depthAttachment)
            {
                auto& depthAttachment = *renderTask.renderPassDesc.depthAttachment;
                auto& viewSlot = m_ResourceAccessor->GetSlot(depthAttachment.textureView);
                auto& texture = m_ResourceAccessor->GetSlot(viewSlot.desc.texture);
                auto* virtualResource=m_Resources[m_AccessIdToResourceIndex[viewSlot.desc.texture.handle]].get();
                if (texture.virtualInfo.layout != rhi::TextureLayout::DepthStencilAttachment)
                {
                    auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                    transitionTask->texture = texture.id;
                    transitionTask->oldLayout = texture.virtualInfo.layout;
                    transitionTask->newLayout = rhi::TextureLayout::DepthStencilAttachment;
                    transitionTask->writes.push_back(virtualResource);
                    transitionTask->tag="Transition"+CreateUniqueId();
                    m_Tasks.push_back(std::move(transitionTask));
                    newSteps.push_back(m_Tasks.back().get());
                    texture.virtualInfo.layout = rhi::TextureLayout::DepthStencilAttachment;
                }
            }
        }
        else if (task.type == TaskType::UploadTextureTask)
        {
            auto& uploadTask = static_cast<UploadTextureTask&>(task);
            auto& sourceSlot = m_ResourceAccessor->GetSlot(uploadTask.source);
            auto& destinationSlot = m_ResourceAccessor->GetSlot(uploadTask.destination);
            if (destinationSlot.virtualInfo.layout != rhi::TextureLayout::TransferDst)
            {
                auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                transitionTask->texture = uploadTask.destination;
                transitionTask->oldLayout = destinationSlot.virtualInfo.layout;
                transitionTask->newLayout = rhi::TextureLayout::TransferDst;
                transitionTask->writes.push_back(m_Resources[m_AccessIdToResourceIndex[uploadTask.destination.handle]].get());
                transitionTask->tag="Transition"+CreateUniqueId();
                m_Tasks.push_back(std::move(transitionTask));
                newSteps.push_back(m_Tasks.back().get());
                destinationSlot.virtualInfo.layout = rhi::TextureLayout::TransferDst;
            }
        }
        else if (task.type == TaskType::DownloadTextureTask)
        {
            auto& downloadTask = static_cast<DownloadTextureTask&>(task);
            auto& sourceSlot = m_ResourceAccessor->GetSlot(downloadTask.source);
            auto& destinationSlot = m_ResourceAccessor->GetSlot(downloadTask.destination);
            if (sourceSlot.virtualInfo.layout != rhi::TextureLayout::TransferSrc)
            {
                auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
                transitionTask->texture = downloadTask.source;
                transitionTask->oldLayout = sourceSlot.virtualInfo.layout;
                transitionTask->newLayout = rhi::TextureLayout::TransferSrc;
                transitionTask->writes.push_back(m_Resources[m_AccessIdToResourceIndex[downloadTask.source.handle]].get());
                transitionTask->tag="Transition"+CreateUniqueId();
                m_Tasks.push_back(std::move(transitionTask));
                newSteps.push_back(m_Tasks.back().get());
                sourceSlot.virtualInfo.layout = rhi::TextureLayout::TransferSrc;
            }
        }
        // add the task to new steps
        newSteps.push_back(&task);
    }
    // restore image layout
    for (auto& resource : m_Resources)
    {
        if (resource->code != ResourceCode::Texture)
        {
            continue;
        }
        auto& texture = static_cast<VirtualResource<rhi::Texture2D>&>(*resource);
        auto& slot = m_ResourceAccessor->GetSlot(texture.id);
        if (slot.virtualInfo.layout != texture.desc.layout)
        {
            auto transitionTask = CreateScope<ImageLayoutTransitionTask>();
            transitionTask->texture = texture.id;
            transitionTask->oldLayout = slot.virtualInfo.layout;
            transitionTask->newLayout = texture.desc.layout;
            transitionTask->tag="Transition"+CreateUniqueId();
            transitionTask->writes.push_back(resource.get());
            m_Tasks.push_back(std::move(transitionTask));
            newSteps.push_back(m_Tasks.back().get());
            slot.virtualInfo.layout = texture.desc.layout;
        }
    }
    m_Steps = std::move(newSteps);
}
void RenderGraph::MergeRenderPasses()
{
    bool inPass = false;
    RenderPassDesc currentPassDesc;
    RenderTaskBase* lastRenderTask = nullptr;
    for (auto* _task : m_Steps)
    {
        auto& task = *_task;
        if (inPass)
        {
            if (task.type == TaskType::RenderTask)
            {
                auto& renderTask = static_cast<RenderTaskBase&>(task);
                if (renderTask.renderPassDesc != currentPassDesc)
                {
                    // 结束上一个render pass
                    lastRenderTask->skipRenderPassEnd = false;
                    // 开始新的render pass
                    currentPassDesc = renderTask.renderPassDesc;
                    lastRenderTask = &renderTask;
                    renderTask.skipRenderPassEnd = true;
                }
                else
                {
                    // 合并render pass
                    renderTask.skipRenderPassBegin = true;
                    renderTask.skipRenderPassEnd = true;
                    lastRenderTask = &renderTask;
                }
            }
            else
            {
                // 结束上一个render pass
                lastRenderTask->skipRenderPassEnd = false;
                inPass = false;
            }
        }
        else
        {
            if (task.type == TaskType::RenderTask)
            {
                // 开始新的render pass
                auto& renderTask = static_cast<RenderTaskBase&>(task);
                inPass = true;
                currentPassDesc = renderTask.renderPassDesc;
                lastRenderTask = &renderTask;
                renderTask.skipRenderPassEnd = true;
            }
        }
    }
    // handle the case last task is a render task
    if (inPass && lastRenderTask)
    {
        lastRenderTask->skipRenderPassEnd = false;
    }
}
void RenderGraph::SetResourceSlotSupportsInFlightResources()
{
    for (auto& resource : m_Resources)
    {
        if (resource->code == ResourceCode::TextureView)
        {
            auto& textureView = static_cast<VirtualResource<rhi::TextureView>&>(*resource);
            auto textureId = textureView.desc.texture;
            assert(m_AccessIdToResourceIndex.contains(textureId.handle) && "Texture resource not found");
            auto& virtualResource = *m_Resources[m_AccessIdToResourceIndex[textureId.handle]];
            assert(virtualResource.code == ResourceCode::Texture);
            if (virtualResource.writers.empty())
            {
                continue;
            }
            m_ResourceAccessor->SetSlotSupportsInFlightResources(textureView.id);
        }
        else
        {
            if (!resource->writers.empty())
            {
                VisitVirtualResource(
                    *resource, [&](auto&& r) -> void { m_ResourceAccessor->SetSlotSupportsInFlightResources(r.id); });
            }
        }
    }
}

void RenderGraph::Execute()
{
    for (auto* _task : m_Steps)
    {
        auto& task = *_task;
        switch (task.type)
        {
        case TaskType::RenderTask: {
            auto& renderTask = static_cast<RenderTaskBase&>(task);
            renderTask.Execute(*m_CommandBuffer, *m_ResourceAccessor);
        }
        break;
        case TaskType::ImageLayoutTransitionTask: {
            auto& transitionTask = static_cast<ImageLayoutTransitionTask&>(task);
            transitionTask.Execute(*m_CommandBuffer, *m_ResourceAccessor);
        }
        break;
        default:
            assert(false && "Unsupported task type in RenderGraph::Execute");
        }
    }
}
namespace
{
class Graphviz
{
public:
    enum class Color
    {
        Blue,
        Red,
        Green,
        Yellow,
        Black,
        White
    };
    std::string result;

    void BeginGraph(const std::string& tag)
    {
        result += "digraph " + tag + " {\n";
    }
    void EndGraph()
    {
        result += "}\n";
    }

    std::string ColorToString(Color color)
    {
        switch (color)
        {
        case Color::Blue:
            return "blue";
        case Color::Red:
            return "red";
        case Color::Green:
            return "green";
        case Color::Yellow:
            return "yellow";
        case Color::Black:
            return "black";
        case Color::White:
            return "white";
        default:
            assert(false && "unknown color");
            return "black";
        }
    }

    constexpr static Color TextureColor = Color::Green;
    constexpr static Color ImageViewColor = Color::Blue;
    constexpr static Color FrameBufferColor = Color::Red;
    constexpr static Color RenderPassColor = Color::Yellow;
    constexpr static Color RenderTaskColor = Color::Red;
    constexpr static Color TransitionTaskColor = Color::Red;
    constexpr static Color ReadEdgeColor = Color::Blue;
    constexpr static Color WriteEdgeColor = Color::Red;
    constexpr static Color CreateEdgeColor = Color::Green;
    constexpr static Color BorderColor = Color::White;

    void AddTaskNode(TaskBase* task)
    {
        switch (task->type)
        {
        case TaskType::RenderTask: {
            auto& t = static_cast<RenderTaskBase&>(*task);
            AddNode(t.tag, BorderColor, RenderTaskColor);
        }
        break;
        case TaskType::ImageLayoutTransitionTask: {
            auto& t = static_cast<ImageLayoutTransitionTask&>(*task);
            AddNode(t.tag, BorderColor, TransitionTaskColor);
        }
        break;
        default:
            assert(false && "Unsupported task type in Graphviz::AddTaskNode");
        }
        AddTaskCreateEdges(task);
        AddTaskReadEdges(task);
        AddTaskWriteEdges(task);
    }

private:
    void AddEdge(const std::string& from, const std::string& to, Color color,const std::string& label)
    {
        result += "  \"" + from + "\" -> \"" + to + "\" [color=" + ColorToString(color)+",label=\""+label+"\"];\n";
    }
    void AddNode(const std::string& tag, Color borderColor, Color fillColor)
    {
        result += "  \"" + tag + "\" [style=filled,fillcolor=" + ColorToString(fillColor)
                  + ",color=" + ColorToString(borderColor) + "];\n";
    }
    std::unordered_map<std::string, int> m_TimelineAlias;
    std::string GetTimelineAlias(const std::string& tag)
    {
        if (m_TimelineAlias.find(tag) == m_TimelineAlias.end())
        {
            m_TimelineAlias[tag] = 0;
        }
        return tag + "$" + std::to_string(m_TimelineAlias[tag]);
    }
    void IncrementTimelineAlias(const std::string& tag)
    {
        if (m_TimelineAlias.find(tag) == m_TimelineAlias.end())
        {
            m_TimelineAlias[tag] = 0;
        }
        m_TimelineAlias[tag]++;
    }

private:
    void AddTaskCreateEdges(TaskBase* task)
    {
        for (auto& resource : task->creates)
        {
            AddEdge(task->tag, GetTimelineAlias(resource->tag), CreateEdgeColor,"Create");
            IncrementTimelineAlias(resource->tag);
        }
    }
    void AddTaskReadEdges(TaskBase* task)
    {
        for (auto& resource : task->reads)
        {
            AddEdge(task->tag, GetTimelineAlias(resource->tag), ReadEdgeColor,"Read");
        }
    }
    void AddTaskWriteEdges(TaskBase* task)
    {
        for (auto& resource : task->writes)
        {
            AddEdge(task->tag, GetTimelineAlias(resource->tag), WriteEdgeColor,"Write");
            IncrementTimelineAlias(resource->tag);
        }
    }
    void AddResourceNode(const std::string& tag, VirtualResourceBase* resource)
    {
        switch (resource->code)
        {
        case ResourceCode::Texture: {
            auto& r = static_cast<VirtualResource<rhi::Texture2D>&>(*resource);
            AddNode(tag, BorderColor, TextureColor);
        }
        break;
        case ResourceCode::TextureView: {
            auto& r = static_cast<VirtualResource<rhi::TextureView>&>(*resource);
            AddNode(tag, BorderColor, ImageViewColor);
        }
        break;
        default:
            assert(false && "Unsupported resource code in Graphviz::AddResourceNode");
        }
    }
};
} // namespace
std::string RenderGraph::ExportGraphviz() const
{
    Graphviz graphviz;
    graphviz.BeginGraph("RenderGraph");

    for (auto& step : m_Steps) 
    {
        graphviz.AddTaskNode(step);
    }
    graphviz.EndGraph();
    return graphviz.result;
}
} // namespace Aether::RenderGraph