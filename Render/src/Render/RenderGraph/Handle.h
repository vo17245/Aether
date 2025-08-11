#pragma once
#include <cstdint>
#include <limits>
#include <cassert>
#include <vector>
#include <Core/Core.h>
namespace Aether::RenderGraph
{
struct Handle
{
    using Id = uint16_t;
    using Version = uint16_t;
    constexpr static inline const Id InvalidId = std::numeric_limits<Id>::max();
    constexpr static inline const Id MaxId = InvalidId;
    Id id;
    Version version;
    Handle(Id id, Version version) :
        id(id), version(version)
    {
    }
    Handle() :
        id(0), version(0)
    {
    }
    bool IsValid() const
    {
        return id != InvalidId;
    }
    bool operator==(const Handle& other) const
    {
        return id == other.id && version == other.version;
    }
    bool operator!=(const Handle& other) const
    {
        return !(*this == other);
    }
    bool operator<(const Handle& other) const
    {
        return id < other.id || (id == other.id && version < other.version);
    }
    static Handle CreateInvalid()
    {
        return Handle{InvalidId, 0};
    }
};

class HandleAllocator
{
public:
    static constexpr inline const Handle::Id MaxId = Handle::InvalidId;
    HandleAllocator() :
        m_Versions(MaxId + 1, 0)
    {
    }
    Handle Allocate()
    {
        if (m_NextId >= MaxId && m_FreeIds.empty())
        {
            assert(false && "resource handle allocator out of resource");
        }
        Handle::Id id;
        if (!m_FreeIds.empty())
        {
            id = m_FreeIds.back();
            m_FreeIds.pop_back();
        }
        else
        {
            id = m_NextId++;
        }

        auto version = m_Versions[id]++;

        return Handle(id, version);
    }
    Handle::Version GetNextVersion(const Handle::Id& id) const
    {
        return m_Versions[id];
    }
    void Free(const Handle& handle)
    {
        if (handle.id >= m_Versions.size())
        {
            assert(false && "resource handle id out of range");
        }
        if (handle.version != (m_Versions[handle.id] - 1))
        {
            assert(false && "resource handle version mismatch");
        }
        m_FreeIds.push_back(handle.id);
    }

private:
    std::vector<Handle::Version> m_Versions;
    std::vector<Handle::Id> m_FreeIds;
    Handle::Id m_NextId = 0;
};
} // namespace Aether::RenderGraph
namespace Aether
{
template <>
struct Hash<RenderGraph::Handle>
{
    size_t operator()(const RenderGraph::Handle& handle) const
    {
        return std::hash<RenderGraph::Handle::Id>()(handle.id) ^ std::hash<RenderGraph::Handle::Version>()(handle.version);
    }
};
} // namespace Aether