#pragma once
#include "AccessId.h"
#include "Resource/ResourceArena.h"
#include "Resource/DeviceTexture.h"
#include "Resource/ResourceLruPool.h"
namespace Aether::RenderGraph
{
template <typename T>
    requires IsResource<T>::value
struct ResourceSlot
{
    template <typename U>
    ResourceSlot(U&& _desc, uint32_t _resourceCount) :
        desc(std::forward(_desc)), resourceCount(_resourceCount)
    {
    }
    ResourceSlot() = default;
    ResourceSlot(const ResourceSlot&) = default;
    ResourceSlot(ResourceSlot&&) = default;
    ResourceSlot& operator=(const ResourceSlot&) = default;
    ResourceSlot& operator=(ResourceSlot&&) = default;

    ResourceDescType<T>::Type desc;
    ResourceId<T> frameResources[Render::Config::InFlightFrameResourceSlots];
    uint32_t resourceCount;
};
template <typename... Ts>
class ResourceAccessorBase
{
public:
    ResourceAccessorBase(Borrow<ResourceArena> resourceArena) :
        m_ResourceArena(resourceArena), m_FrameIndex(0)
    {
    }
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    ResourceType* GetResource(AccessId<ResourceType> id)
    {
        auto& slots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto iter = slots.m_SlotMap.find(id);
        if (iter == slots.m_SlotMap.end())
        {
            assert(false && "Resource slot not found");
            return nullptr;
        }
        ResourceSlot<ResourceType>& slot = *(*iter->second);
        ResourceId<ResourceType> resourceId = slot.frameResources[m_FrameIndex % slot.resourceCount];
        return m_ResourceArena->GetResource(resourceId);
    }
    template <typename ResourceType>
        requires IsResource<ResourceType>::value
    void Forward(AccessId<ResourceType> from, AccessId<ResourceType> to)
    {
        auto& slots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto iter_from = slots.m_SlotMap.find(from);
        if (iter_from == slots.m_SlotMap.end())
        {
            assert(false && "Resource slot not found");
            return;
        }
        auto iter_to = slots.m_SlotMap.find(to);
        if (iter_to == slots.m_SlotMap.end())
        {
            assert(false && "Resource slot not found");
            return;
        }
        slots.m_Slots.erase(*iter_to->second);
        *iter_to->second = *iter_from->second;
    }
    template <typename ResourceType, typename ResourceDescType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> CreateSlot(ResourceDescType&& desc,bool readOnly)
    {
        auto& indexedSlots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto& slots = indexedSlots.m_Slots;
        auto& slotMap = indexedSlots.m_SlotMap;
        AccessId<ResourceType> id = m_Allocator.Allocate<ResourceType>();
        uint32_t resourceCount=readOnly?1:Render::Config::MaxFramesInFlight;
        ResourceSlot<ResourceType> slot(std::forward(desc),resourceCount);
        slots.push_back(slot);
        auto iter = std::prev(slots.end());
        slotMap[id] = iter;
    }
    bool RealizeResource()
    {
        return false;//Not implemented yet
    }

private:
    Borrow<ResourceArena> m_ResourceArena;
    uint32_t m_FrameIndex; // current frame index
    template <typename ResourceType>
    struct IndexedSlot
    {
        std::list<ResourceSlot<ResourceType>> m_Slots;
        std::unordered_map<AccessId<ResourceType>, typename std::list<ResourceSlot<ResourceType>>::iterator, Hash<AccessId<ResourceType>>> m_SlotMap;
    };
    std::tuple<IndexedSlot<Ts>...> m_Slots;
    AccessIdAllocator m_Allocator;
};
using ResourceAccessor = ResourceAccessorBase<DeviceTexture, DeviceImageView, DeviceFrameBuffer, DeviceBuffer>;
} // namespace Aether::RenderGraph