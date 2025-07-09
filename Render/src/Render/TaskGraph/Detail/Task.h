#pragma once
#include <vector>
#include <functional>
namespace Aether::TaskGraph
{
class ResourceBase;
class TaskBuilder;
class TaskBase
{
public:
    virtual ~TaskBase()=default;
    virtual void Setup(TaskBuilder& builder) = 0;
    virtual void Execute() = 0;
    std::vector<ResourceBase*> creates;
    std::vector<ResourceBase*> reads;
    std::vector<ResourceBase*> writes;
};
template<typename TaskData>
class Task:public TaskBase
{
public:
    Task(std::function<void(TaskBuilder& )> setup,std::function<void(TaskData&)> execute)
    :m_Execute(execute),m_Setup(setup){}
    void Execute()override
    {
        m_Execute(m_Data);
    }
    void Setup(TaskBuilder& builder)
    {
        m_Setup(builder);
    }
private:
    TaskData m_Data;
    std::function<void(TaskData&)> m_Execute;
    std::function<void(TaskBuilder& )> m_Setup;
};
} // namespace Aether::TaskGraph