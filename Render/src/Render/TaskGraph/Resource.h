#pragma once
#include <Render/RenderApi.h>
#include "Realize.h"
#include "Debug.h"
#include "ResourceType.h"
namespace Aether::TaskGraph
{
class DeviceTaskBase;
class ResourceBase
{
public:
    ResourceBase(const std::string& _tag,DeviceTaskBase* creator)
    {
#if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
        tag = _tag;
#endif
        this->creator = creator;
    }
    virtual ~ResourceBase() = default;
    virtual void Realize() = 0;
    virtual void Derealize() = 0;
    bool Transient()const
    {
        return creator!=nullptr;
    }
    DeviceTaskBase* creator = nullptr;
    std::vector<DeviceTaskBase*> readers;
    std::vector<DeviceTaskBase*> writers;
    size_t refCount = 0; // for task graph compilation
#if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
    std::string tag;
#endif
    ResourceType type = ResourceType::Unknown;

};
template<typename Desc>
struct GetResourceType;
template <typename Actual, typename Desc>
class Resource : public ResourceBase
{
public:
    Resource(const std::string& tag,DeviceTaskBase* creator ,const Desc& desc) :
        ResourceBase(tag,creator),m_Desc(desc)
    {
        type = GetResourceType<Desc>::type;
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
    void SetActual(Scope<Actual>&& actual)
    {
        m_Actual = std::move(actual);
    }
    const Desc& GetDesc()const
    {
        return m_Desc;
    }
    Scope<Actual>& GetActual()
    {
        return m_Actual;
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
