#pragma once
#include "Resource.h"
#include <Core/Cache.h>
#include "ResourceImpl.h"
namespace Aether::TaskGraph
{

struct ResourceArenaHandle
{
    Handle handle;
};
class ResourceArena
{
public:
    ResourceArena():m_TexturePool(1000),m_Resources(Handle::MaxId+1){}

    ResourceArenaHandle ImportTexture(const TextureDesc& desc,Scope<Texture>&& texture)
    {
        ResourceArenaHandle handle{m_ResourceHandleAllocator.Allocate()};
        m_Resources[handle.handle.id] = std::move(texture);
        m_TexturePool.Push(desc, handle.handle);
        return handle;
    }
private:
    // desc -> multiple id(m_Resources index) 
    LruPool<TextureDesc, Handle::Id,Hash<TextureDesc>> m_TexturePool;
    std::vector<Scope<ResourceBase>> m_Resources;
    HandleAllocator m_ResourceHandleAllocator;
};

class ResourcePool
{
public:
    struct ResourceSlot
    {
        using Id= Handle::Id;
        // resource index in m_Resources for each frame slot
        std::array<Id, Render::Config::InFlightFrameResourceSlots> frameSlots;
        uint32_t resouceCount=1;
        Id GetCurrentFrameResourceId(uint32_t currentFrameIndex) const
        {
            return frameSlots[currentFrameIndex % resouceCount];
        }
        ResourceSlot()
        {
            for (auto& slot : frameSlots)
            {
                slot = Handle::InvalidId;
            }
        }
    };
    struct InternalHandle
    {
        Handle handle;
    };

private:
    InternalHandle AllocateInternalHandle()
    {
        InternalHandle internalHandle{m_InternalHandleAllocator.Allocate()};
        return internalHandle;
    }
    void FreeInternalHandle(const InternalHandle& handle)
    {
        m_InternalHandleAllocator.Free(handle.handle);
    }
private:
    std::vector<Scope<ResourceBase>> m_Resources;
    std::vector<ResourceSlot> m_Slots;
    HandleAllocator m_InternalHandleAllocator;
};
} // namespace Aether::TaskGraph