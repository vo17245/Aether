#pragma once
#include <Render/RenderApi.h>
#include "Realize.h"
#include "Debug.h"
#include "ResourceType.h"
#include <limits>
#ifdef max
#undef max
#endif
namespace Aether::TaskGraph
{
struct ResourceHandle
{
    using Id = uint16_t;
    using Version = uint16_t;
    constexpr static inline const Version InvalidVersion = std::numeric_limits<Version>::max();
    constexpr static inline const Id InvalidId = std::numeric_limits<Id>::max();
    Id id;
    Version version;
    ResourceHandle(Id id, Version version) :
        id(id), version(version)
    {
    }
    ResourceHandle() :
        id(0), version(InvalidVersion)
    {
    }
    bool IsValid() const
    {
        return version != InvalidVersion;
    }
    bool operator==(const ResourceHandle& other) const
    {
        return id == other.id && version == other.version;
    }
    bool operator!=(const ResourceHandle& other) const
    {
        return !(*this == other);
    }
    bool operator<(const ResourceHandle& other) const
    {
        return id < other.id || (id == other.id && version < other.version);
    }
};
// typed resource handle
template <typename ResourceType>
struct ResourceId
{
    ResourceHandle handle;
    ResourceId(ResourceHandle handle) :
        handle(handle)
    {
    }
};
class ResourceHandleAllocator
{
public:
    static constexpr inline const ResourceHandle::Id MaxResourceId = ResourceHandle::InvalidId;
    ResourceHandleAllocator() :
        m_Versions(MaxResourceId + 1, 0)
    {
    }
    ResourceHandle Allocate()
    {
        if (m_NextId >= MaxResourceId)
        {
            assert(false && "resource handle allocator out of resource");
            return ResourceHandle();
        }
        auto version = m_Versions[m_NextId]++;
        if (version == ResourceHandle::InvalidVersion)
        {
            assert(false && "resource handle version overflow");
            return ResourceHandle();
        }
        return ResourceHandle(m_NextId++, version);
    }
    ResourceHandle::Version GetCurrentVersion(const ResourceHandle::Id& id) const
    {
        return m_Versions[id];// equal to 
    }

private:
    std::vector<ResourceHandle::Version> m_Versions;
    ResourceHandle::Id m_NextId = 0;
};
class RenderTaskBase;
class ResourceBase
{
public:
    ResourceBase(const std::string& _tag, RenderTaskBase* creator)
    {
#if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
        tag = _tag;
#endif
        this->creator = creator;
    }
    virtual ~ResourceBase() = default;
    virtual void Realize() = 0;
    virtual void Derealize() = 0;
    bool Transient() const
    {
        return creator != nullptr;
    }
    RenderTaskBase* creator = nullptr;
    std::vector<RenderTaskBase*> readers;
    std::vector<RenderTaskBase*> writers;
    size_t refCount = 0; // for task graph compilation
#if AETHER_RENDER_TASKGRAPH_DEBUG_ENABLE_DEBUG_TAG
    std::string tag;
#endif
    ResourceType type = ResourceType::Unknown;
};
template <typename Desc>
struct GetResourceType;
template <typename Actual, typename Desc>
class Resource : public ResourceBase
{
public:
    Resource(const std::string& tag, RenderTaskBase* creator, const Desc& desc) :
        ResourceBase(tag, creator), m_Desc(desc)
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
    const Desc& GetDesc() const
    {
        return m_Desc;
    }
    Desc& GetDesc()
    {
        return m_Desc;
    }
    /**
     * @note check a resource holds a actual or borrow by Transient() function
     * if Transient() is true, call GetActual() to get the actual resource
     * if Transient() is false, call GetActualBorrow() to get the borrow resource
     */
    Scope<Actual>& GetActualScope()
    {
        assert(m_Actual && "resource not realized");
        return m_Actual;
    }
    Borrow<Actual> GetActualBorrow()
    {
        assert(m_ActualBorrow && "resource not realized");
        return m_ActualBorrow;
    }
    void SetActualBorrow(Borrow<Actual> actual)
    {
        m_ActualBorrow = actual;
    }
    Actual* GetActual()
    {
        if (m_Actual)
        {
            return m_Actual.get();
        }
        else if (m_ActualBorrow)
        {
            return m_ActualBorrow;
        }
        assert(false && "resource not realized or borrowed");
        return nullptr;
    }

private:
    Scope<Actual> m_Actual;           // for transient resource in task graph, or owned external resource resource
    Actual* m_ActualBorrow = nullptr; // for not owned external resource, just a borrow, task graph will *NOT* manage its lifetime
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
