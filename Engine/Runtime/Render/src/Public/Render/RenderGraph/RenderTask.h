#pragma once
#include "Resource/ResourceId.h"
#include "Resource/ResourceAccessor.h"
#include "TaskBase.h"
#include "Resource/Attachment.h"
#include <Render/RHI.h>
namespace Aether::RenderGraph
{

struct RenderPassDesc
{
    Attachment colorAttachment[rhi::MaxColorAttachments];
    std::optional<Attachment> depthAttachment;
    size_t colorAttachmentCount = 0;            // number of color attachments
    Vec4f clearColor[rhi::MaxColorAttachments]; // default clear color
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
        if (width != other.width)
        {
            return false;
        }
        if (height != other.height)
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
    virtual void Execute(rhi::CommandList& commandBuffer, ResourceAccessor& resourceAccessor) = 0;
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
    AccessId<ResourceType> Create(const std::string& tag, const typename ResourceDescType<ResourceType>::Type& desc);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Write(AccessId<ResourceType> resourceId);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Read(AccessId<ResourceType> resourceId);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Write(const std::string& tag);
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> Read(const std::string& tag);
    inline RenderGraph& GetGraph()
    {
        return m_Graph;
    }
    inline const RenderGraph& GetGraph() const
    {
        return m_Graph;
    }

private:
    Borrow<RenderTaskBase> m_Task;
    RenderGraph& m_Graph;
};
template <typename TaskDataType>
struct RenderTask : public RenderTaskBase
{
    TaskDataType data;
    std::function<void(rhi::CommandList&, ResourceAccessor&, TaskDataType&)> execute;
    virtual void Execute(rhi::CommandList& cmdList, ResourceAccessor& resourceAccessor)
    {
        if (!skipRenderPassBegin)
        {
            uint16_t clearValueCount = renderPassDesc.colorAttachmentCount;
            VkClearValue clearValues[rhi::MaxColorAttachments + 1];
            for (size_t i = 0; i < renderPassDesc.colorAttachmentCount; ++i)
            {
                clearValues[i].color = {renderPassDesc.clearColor[i].x(), renderPassDesc.clearColor[i].y(),
                                        renderPassDesc.clearColor[i].z(), renderPassDesc.clearColor[i].w()};
            }
            if (renderPassDesc.depthAttachment)
            {
                clearValues[clearValueCount].depthStencil.depth = renderPassDesc.clearDepth;
                clearValues[clearValueCount].depthStencil.stencil = renderPassDesc.clearStencil;
                clearValueCount++;
            }
            auto renderPass = CreateRHIRenderPass(renderPassDesc, resourceAccessor);
            cmdList.BeginRenderPass(renderPass);
        }
        execute(cmdList, resourceAccessor, data);
        if (!skipRenderPassEnd)
        {
            cmdList.EndRenderPass();
        }
    }
    static rhi::RenderPass CreateRHIRenderPass(const RenderPassDesc& desc, ResourceAccessor& resourceAccessor)
    {
        auto renderPass = rhi::RenderPass{};
        renderPass.colorAttachments.reserve(desc.colorAttachmentCount);
        for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            auto& colorAttachment = desc.colorAttachment[i];
            auto* imageView = resourceAccessor.GetResource(colorAttachment.textureView);
            assert(imageView && "Failed to get image view resource");
            renderPass.colorAttachments.emplace_back(
                imageView, colorAttachment.loadOp, colorAttachment.storeOp, desc.clearColor[i]);
        }
        if (desc.depthAttachment)
        {
            auto& depthAttachment = *desc.depthAttachment;
            auto* imageView = resourceAccessor.GetResource(depthAttachment.textureView);
            assert(imageView && "Failed to get image view resource");
            renderPass.depthAttachment = {
                imageView, depthAttachment.loadOp, depthAttachment.storeOp, desc.clearDepth, desc.clearStencil};
        }
        return renderPass;
    }
};

} // namespace Aether::RenderGraph