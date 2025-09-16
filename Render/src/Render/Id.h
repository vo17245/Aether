#pragma once
#include <Core/Core.h>
namespace Aether::Render
{
struct Handle
{
    using Id = uint16_t;
    using Version = uint16_t;
    constexpr static inline const Id InvalidId = std::numeric_limits<Id>::max();
    constexpr static inline const Id MaxId = InvalidId;
    Id id;
    Version version;
    Handle(Id id, Version version) : id(id), version(version)
    {
    }
    Handle() : id(InvalidId), version(0)
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
    operator bool() const
    {
        return IsValid();
    }
};

class HandleAllocator
{
public:
    static constexpr inline const Handle::Id MaxId = Handle::InvalidId;

    static Handle Allocate()
    {
        return GetSingleton().AllocateImpl();
    }
   
    static void Free(const Handle& handle)
    {
        GetSingleton().FreeImpl(handle);
    }
    static bool IsHandleFreed(const Handle& handle)
    {
        return GetSingleton().IsHandleFreedImpl(handle);
    }

private:
    static HandleAllocator& GetSingleton();
    Handle AllocateImpl()
    {
        if (m_NextId >= MaxId && m_FreeIds.empty())
        {
            assert(false && "resource handle allocator out of resource");
            return Handle::CreateInvalid();
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
    
    void FreeImpl(const Handle& handle)
    {
        if (handle.id >= m_Versions.size())
        {
            assert(false && "resource handle id out of range");
        }
        if (handle.version != (m_Versions[handle.id] - 1))
        {
            assert(false && "resource handle version mismatch");
        }
        m_Versions[handle.id]++;   
        m_FreeIds.push_back(handle.id);
    }
    bool IsHandleFreedImpl(const Handle& handle)const
    {
        return m_Versions[handle.id] - 1 != handle.version;
    }

private:
    HandleAllocator() : m_Versions(MaxId, 0)
    {
    }

private:
    std::vector<Handle::Version> m_Versions;
    std::vector<Handle::Id> m_FreeIds;
    Handle::Id m_NextId = 0;
};
// typed resource id
template <typename T>
struct Id
{
    Handle handle;
    bool operator==(const Id<T>& other) const
    {
        return handle == other.handle;
    }
    bool operator!=(const Id<T>& other) const
    {
        return !(*this == other);
    }
    operator bool() const
    {
        return handle.IsValid();
    }
};
class IdAllocator
{
public:
    template <typename T>
    static Id<T> Allocate()
    {
        Handle handle = HandleAllocator::Allocate();
        return Id<T>{handle};
    }
    template <typename T>
    static void Free(const Id<T>& id)
    {
        HandleAllocator::Free(id.handle);
    }
    template <typename T>
    static bool IsIdFreed(const Id<T>& id)
    {
        return HandleAllocator::IsHandleFreed(id.handle);
    }

private:
    IdAllocator() = default;
};

} // namespace Aether::Render

namespace Aether
{
template <>
struct Hash<Render::Handle>
{
    size_t operator()(const Render::Handle& handle) const
    {
        return HashCombine(std::hash<Render::Handle::Id>()(handle.id), std::hash<Render::Handle::Version>()(handle.version));
    }
};
} // namespace Aether