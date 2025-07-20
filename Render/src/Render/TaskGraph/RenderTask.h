#pragma once
#include "TaskBase.h"
#include "ResourceImpl.h"
namespace Aether::TaskGraph
{
struct Attachment
{
    Texture* texture = nullptr;
    DeviceAttachmentLoadOp loadOp;
    DeviceAttachmentStoreOp storeOp;
    bool operator==(const Attachment& other)const
    {
        return texture==other.texture && loadOp==other.loadOp && storeOp==other.storeOp;
    }
    bool operator!=(const Attachment& other)const
    {
        return !(other == *this);
    }
};
struct RenderPassDesc
{
    Attachment colorAttachment[DeviceRenderPassDesc::MaxColorAttachments];
    Attachment depthAttachment;
    bool operator==(const RenderPassDesc& other)
    {
        for(size_t i=0;i<DeviceRenderPassDesc::MaxColorAttachments;++i)
        {
            if(colorAttachment[i]!=other.colorAttachment[i])
            {
                return false;
            }
        }
        return depthAttachment==other.depthAttachment;

    }
};
class RenderTaskBase : public TaskBase
{
public:
    virtual ~RenderTaskBase() = default;
    virtual void Setup(TaskBuilder& builder) = 0;
    virtual void Execute(DeviceCommandBuffer& commandBuffer) = 0;
};

template <typename TaskData>
class RenderTask : public RenderTaskBase
{
public:
    RenderTask(const RenderPassDesc& renderPassDesc,
               std::function<void(TaskData&, TaskBuilder&)> setup,
               std::function<void(TaskData&, DeviceCommandBuffer& commandBuffer)> execute) :
        m_Execute(execute), m_Setup(setup),m_RenderPassDesc(renderPassDesc)
    {
    }
    void Execute(DeviceCommandBuffer& commandBuffer) override
    {
        m_Execute(m_Data, commandBuffer);
    }
    void Setup(TaskBuilder& builder) override
    {
        m_Setup(m_Data, builder);
    }
    TaskData& GetData()
    {
        return m_Data;
    }

private:
    TaskData m_Data;
    std::function<void(TaskData&, DeviceCommandBuffer& commandBuffer)> m_Execute;
    std::function<void(TaskBuilder&, TaskData&)> m_Setup;
    RenderPassDesc m_RenderPassDesc;
};
} // namespace Aether::TaskGraph