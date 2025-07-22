#pragma once
#include "TaskBase.h"
#include "ResourceImpl.h"
namespace Aether::TaskGraph
{

class RenderTaskBase : public TaskBase
{
public:
    RenderTaskBase(const RenderPassDesc& renderPassDesc) :
        m_RenderPassDesc(renderPassDesc)
    {
    }
    virtual ~RenderTaskBase() = default;
    virtual void Setup(TaskBuilder& builder) = 0;
    virtual void Execute(DeviceCommandBuffer& commandBuffer) = 0;
    const RenderPassDesc& GetRenderPassDesc() const
    {
        return m_RenderPassDesc;
    }
private:
    RenderPassDesc m_RenderPassDesc;
};

template <typename TaskData>
class RenderTask : public RenderTaskBase
{
public:
    RenderTask(const RenderPassDesc& renderPassDesc,
               std::function<void(TaskData&, TaskBuilder&)> setup,
               std::function<void(TaskData&, DeviceCommandBuffer& commandBuffer)> execute) :
        m_Execute(execute), m_Setup(setup),RenderTaskBase(renderPassDesc)
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
    
};
} // namespace Aether::TaskGraph