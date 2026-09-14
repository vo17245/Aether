#pragma once
#include <cstdint>
#include <limits>
#include <cassert>
#include <vector>
#include <optional>
#include <stdexcept>
#include <Core/Core.h>
namespace Aether::RenderGraph
{
struct Handle
{
    using Id = uint16_t;
    using Version = uint32_t;
    constexpr static inline const Id InvalidId = std::numeric_limits<Id>::max();
    constexpr static inline const Id MaxId = InvalidId;
    Id id;
    Version version;
    Handle(Id id, Version version) :
        id(id), version(version)
    {
    }
    Handle() :
        id(InvalidId), version(0)
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
        m_Versions(MaxId, 0), m_Active(MaxId, false), m_Retired(MaxId, false)
    {
        m_FreeIds.reserve(MaxId);
    }
    size_t RemainingCapacity() const
    {
        return (size_t(MaxId) - m_NextId) + m_FreeIds.size();
    }
    std::optional<Handle> TryAllocate()
    {
        if (RemainingCapacity() == 0) return std::nullopt;
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

        m_Active[id] = true;
        return Handle(id, m_Versions[id]);
    }
    Handle Allocate()
    {
        auto handle = TryAllocate();
        if (!handle) throw std::overflow_error("resource handle allocator exhausted");
        return *handle;
    }
    Handle::Version GetNextVersion(const Handle::Id& id) const
    {
        return id < MaxId ? m_Versions[id] : 0;
    }
    bool IsActive(const Handle& handle) const
    {
        return handle.id < m_NextId && !m_Retired[handle.id] &&
               m_Active[handle.id] && m_Versions[handle.id] == handle.version;
    }
    bool Free(const Handle& handle)
    {
        if (!IsActive(handle)) return false;
        m_Active[handle.id] = false;
        if (m_Versions[handle.id] == std::numeric_limits<Handle::Version>::max())
            m_Retired[handle.id] = true;
        else {
            ++m_Versions[handle.id];
            m_FreeIds.push_back(handle.id);
        }
        return true;
    }

private:
    std::vector<Handle::Version> m_Versions;
    std::vector<bool> m_Active;
    std::vector<bool> m_Retired;
    std::vector<Handle::Id> m_FreeIds;
    uint32_t m_NextId = 0;
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
