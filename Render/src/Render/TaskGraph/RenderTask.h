#pragma once
#include "TaskBase.h"
#include "ResourceImpl.h"
#include "TaskBuilder.h"
#include "RenderPass.h"
#include "ResourceAccessor.h"
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
    virtual void Setup(RenderTaskBuilder& builder) = 0;
    virtual void Execute(DeviceCommandBufferView& commandBuffer,ResourceAccessor& resourceAccessor) = 0;
    const RenderPassDesc& GetRenderPassDesc() const
    {
        return m_RenderPassDesc;
    }
    RenderPassDesc& GetRenderPassDesc()
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
               std::function<void(RenderTaskBuilder&,TaskData&)>&& setup,
               std::function<void(TaskData&, DeviceCommandBufferView& ,ResourceAccessor&)>&& execute) :
        m_Execute(std::move(execute)), m_Setup(std::move(setup)),RenderTaskBase(renderPassDesc)
    {
    }
    void Execute(DeviceCommandBufferView& commandBuffer,ResourceAccessor& resourceAccessor) override
    {
        m_Execute(m_Data, commandBuffer,resourceAccessor);
    }
    void Setup(RenderTaskBuilder& builder) override
    {
        m_Setup( builder,m_Data);
    }
    TaskData& GetData()
    {
        return m_Data;
    }

private:
    TaskData m_Data;
    std::function<void(TaskData&, DeviceCommandBufferView& ,ResourceAccessor&)> m_Execute;
    std::function<void(RenderTaskBuilder&,TaskData&)> m_Setup;
    
};
} // namespace Aether::TaskGraph