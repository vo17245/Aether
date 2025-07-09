#pragma once
#include <Render/RenderApi.h>
#include "Realize.h"
#include "Debug.h"
namespace Aether::TaskGraph
{
class TaskBase;
class ResourceBase
{
public:
    ResourceBase(const std::string& tag,TaskBase* creator);
    virtual ~ResourceBase() = default;
    virtual void Realize() = 0;
    virtual void Derealize() = 0;
    TaskBase* creator = nullptr;
    std::vector<TaskBase*> readers;
    std::vector<TaskBase*> writers;
    size_t refCnt = 0; // for task graph compilation
#if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
    std::string m_Tag;
#endif
};
template <typename Actual, typename Desc>
class Resource : public ResourceBase
{
#if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
    Resource(const std::string& tag,TaskBase* creator ,const Desc& desc) :
        ResourceBase(tag,creator),m_Desc(desc)
    {
    }
#endif
    Resource(const Desc& desc) :
        m_Desc(desc)
    {
    }
    void Realize() override
    {
        m_Actual = ::Aether::TaskGraph::Realize<Actual, Desc>(m_Desc);
        assert(m_Actual && "failed to realize resource");
    }
    void Derealize() override
    {
        m_Actual.reset();
    }

private:
    Scope<Actual> m_Actual;
    Desc m_Desc;
};
template <typename T>
struct Hash
{
    size_t operator()(const T& value) const
    {
        return std::hash<T>()(value);
    }
};

} // namespace Aether::TaskGraph
