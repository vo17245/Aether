#pragma once
#include <Render/RenderApi.h>
#include "Resource/DeviceTexture.h"
#include "Resource/ResourceId.h"
#include "ResourceAccessor.h"
#include "TaskBase.h"
namespace Aether::RenderGraph
{
struct Attachment
{
    ResourceId<DeviceImageView> texture;
    DeviceAttachmentLoadOp loadOp;
    DeviceAttachmentStoreOp storeOp;
    bool operator==(const Attachment& other) const
    {
        return texture == other.texture && loadOp == other.loadOp && storeOp == other.storeOp;
    }
    bool operator!=(const Attachment& other) const
    {
        return !(other == *this);
    }
};
struct RenderPassDesc
{
    Attachment colorAttachment[DeviceRenderPassDesc::MaxColorAttachments];
    std::optional<Attachment> depthAttachment;
    size_t colorAttachmentCount = 0;             // number of color attachments
    Vec4f clearColor = {0.0f, 1.0f, 0.0f, 1.0f}; // default clear color
    bool operator==(const RenderPassDesc& other) const
    {
        if (colorAttachmentCount != other.colorAttachmentCount)
        {
            return false;
        }
        if (clearColor != other.clearColor)
        {
            return false;
        }
        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (colorAttachment[i] != other.colorAttachment[i])
            {
                return false;
            }
        }
        return depthAttachment == other.depthAttachment;
    }
};
struct RenderTask;
struct RenderGraph;
class RenderTaskBuilder
{
public:
    RenderTaskBuilder(RenderTask& task,RenderGraph& graph):
        m_Task(task),m_Graph(graph)
    {
    }
    RenderTaskBuilder& SetRenderPassDesc(const RenderPassDesc& desc);
    template<typename ResourceType>
    AccessId<ResourceType> Create(const ResourceDescType<ResourceType>::Type& desc);
    template<typename ResourceType>
    AccessId<ResourceType> Write(ResourceId<ResourceType> resourceId);
    template<typename ResourceType>
    AccessId<ResourceType> Read(ResourceId<ResourceType> resourceId);
    
private:
    RenderTask& m_Task;
    RenderGraph& m_Graph;
};
struct RenderTask:public TaskBase
{
    RenderPassDesc renderPassDesc;
    bool skipRenderPassBegin=false;
    bool skipRenderPassEnd=false;
    std::function<void(DeviceCommandBuffer& ,ResourceAccessor& )> execute;
};
inline RenderTaskBuilder& RenderTaskBuilder::SetRenderPassDesc(const RenderPassDesc& desc)
{
    m_Task.renderPassDesc = desc;
    return *this;
}
} // namespace Aether::RenderGraph