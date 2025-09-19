#pragma once
#include <Render/RenderApi.h>
#include "Resource/DeviceTexture.h"
#include "Resource/ResourceId.h"
#include "Resource/ResourceAccessor.h"
#include "TaskBase.h"
#include "Resource/Attachment.h"
namespace Aether::RenderGraph
{

struct RenderPassDesc
{
    Attachment colorAttachment[DeviceRenderPassDesc::MaxColorAttachments];
    std::optional<Attachment> depthAttachment;
    size_t colorAttachmentCount = 0;                             // number of color attachments
    Vec4f clearColor[DeviceRenderPassDesc::MaxColorAttachments]; // default clear color
    float clearDepth;
    uint32_t clearStencil;
    uint32_t width;
    uint32_t height;
    bool operator==(const RenderPassDesc& other) const
    {
        if (colorAttachmentCount != other.colorAttachmentCount)
        {
            return false;
        }
        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (!(clearColor[i] == other.clearColor[i]))
            {
                return false;
            }
        }
        for (size_t i = 0; i < colorAttachmentCount; ++i)
        {
            if (colorAttachment[i] != other.colorAttachment[i])
            {
                return false;
            }
        }
        if (clearDepth != other.clearDepth)
        {
            return false;
        }
        if (clearStencil != other.clearStencil)
        {
            return false;
        }
        if(width!=other.width)
        {
            return false;
        }
        if(height!=other.height)
        {
            return false;
        }
        return depthAttachment == other.depthAttachment;
    }
};
struct RenderTaskBase : public TaskBase
{
    RenderTaskBase() : TaskBase(TaskType::RenderTask)
    {
    }
    virtual ~RenderTaskBase() = default;

    RenderPassDesc renderPassDesc;
    bool skipRenderPassBegin = false;
    bool skipRenderPassEnd = false;
    AccessId<DeviceRenderPass> renderPass;   // set when compile
    AccessId<DeviceFrameBuffer> frameBuffer; // set when compile
    virtual void Execute(DeviceCommandBuffer& commandBuffer, ResourceAccessor& resourceAccessor) = 0;
};
class RenderGraph;
class RenderTaskBuilder
{
public:
    RenderTaskBuilder(Borrow<RenderTaskBase> task, RenderGraph& graph) : m_Task(task), m_Graph(graph)
    {
    }
    RenderTaskBuilder& SetRenderPassDesc(const RenderPassDesc& desc);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Create(const std::string& tag,const typename ResourceDescType<ResourceType>::Type& desc);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Write(AccessId<ResourceType> resourceId);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Read(AccessId<ResourceType> resourceId);

private:
    Borrow<RenderTaskBase> m_Task;
    RenderGraph& m_Graph;
};
template <typename TaskDataType>
struct RenderTask : public RenderTaskBase
{
    TaskDataType data;
    std::function<void(DeviceCommandBuffer&, ResourceAccessor&, TaskDataType&)> execute;
    virtual void Execute(DeviceCommandBuffer& commandBuffer, ResourceAccessor& resourceAccessor)
    {
        auto& cb = commandBuffer.GetVk();
        if (!skipRenderPassBegin)
        {
            uint16_t clearValueCount=renderPassDesc.colorAttachmentCount;
            VkClearValue clearValues[DeviceRenderPassDesc::MaxColorAttachments+1];
            for (size_t i = 0; i < renderPassDesc.colorAttachmentCount; ++i)
            {
                clearValues[i].color = {renderPassDesc.clearColor[i].x(), renderPassDesc.clearColor[i].y(),
                                        renderPassDesc.clearColor[i].z(), renderPassDesc.clearColor[i].w()};
            }
            if(renderPassDesc.depthAttachment)
            {
                clearValues[clearValueCount].depthStencil.depth = renderPassDesc.clearDepth;
                clearValues[clearValueCount].depthStencil.stencil = renderPassDesc.clearStencil;
                clearValueCount++;
            }
            auto& renderPassActual = *resourceAccessor.GetResource(renderPass);
            auto& frameBufferActual = *resourceAccessor.GetResource(frameBuffer);
            cb.BeginRenderPass(renderPassActual.GetVk(), frameBufferActual.GetVk(),
                               std::span<VkClearValue>(clearValues, clearValueCount));
        }
        execute(commandBuffer, resourceAccessor, data);
        if (!skipRenderPassEnd)
        {
            cb.EndRenderPass();
        }
    }
};

} // namespace Aether::RenderGraph