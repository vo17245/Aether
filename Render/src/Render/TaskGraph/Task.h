#pragma once
#include <vector>
#include <functional>
#include <Render/RenderApi.h>
namespace Aether::TaskGraph
{
class ResourceBase;
class TaskBuilder;
class TaskBase
{
public:
    virtual ~TaskBase() = default;
    std::vector<ResourceBase*> creates;
    std::vector<ResourceBase*> reads;
    std::vector<ResourceBase*> writes;
    size_t refCount = 0; // for task graph compilation
    /**
     * @note
     * if false, task will be culled if not affect retained resources
     * if true, task will not be culled even if it does not affect retained resources
    */ 
    bool cullImmune = false; 
};
class DeviceTaskBase : public TaskBase
{
public:
    virtual ~DeviceTaskBase() = default;
    virtual void Setup(TaskBuilder& builder) = 0;
    virtual void Execute(DeviceCommandBuffer& commandBuffer) = 0;
};

template <typename TaskData>
class DeviceTask : public DeviceTaskBase
{
public:
    DeviceTask(std::function<void(TaskData&, TaskBuilder&)> setup, std::function<void(TaskData&, DeviceCommandBuffer& commandBuffer)> execute) :
        m_Execute(execute), m_Setup(setup)
    {
    }
    void Execute(DeviceCommandBuffer& commandBuffer) override
    {
        m_Execute(m_Data);
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
    std::function<void(TaskData&)> m_Execute;
    std::function<void(TaskBuilder&, TaskData&)> m_Setup;
};
} // namespace Aether::TaskGraph