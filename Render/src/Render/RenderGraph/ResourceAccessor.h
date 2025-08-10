#pragma once
#include "AccessId.h"
#include "Resource/ResourceArena.h"
#include "Resource/DeviceTexture.h"
#include "Resource/ResourceLruPool.h"
#include "Resource/DeviceBuffer.h"
namespace Aether::RenderGraph
{
template <typename T>
    requires IsResource<T>::value
struct ResourceSlot
{
    template <typename U>
    ResourceSlot(U&& _desc) :
        desc(std::forward(_desc))
    {
    }
    ResourceSlot() = default;
    ResourceSlot(const ResourceSlot&) = default;
    ResourceSlot(ResourceSlot&&) = default;
    ResourceSlot& operator=(const ResourceSlot&) = default;
    ResourceSlot& operator=(ResourceSlot&&) = default;

    ResourceDescType<T>::Type desc;
    ResourceId<T> frameResources[Render::Config::InFlightFrameResourceSlots];
    uint32_t resourceCount = false; 
    bool isRealized[Render::Config::InFlightFrameResourceSlots] = {false}; // if the resource is realized
};
template <typename... Ts>
class ResourceAccessorBase
{
public:
    ResourceAccessorBase(Borrow<ResourceArena> resourceArena,Borrow<ResourceLruPool> resourcePool) :
        m_ResourceArena(resourceArena), m_ResourcePool(resourcePool),m_FrameIndex(0)
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
        uint32_t index=m_FrameIndex % slot.resourceCount;
        if(!slot.isRealized[index])
        {
            slot.frameResources[index]=m_ResourcePool->PopOrCreate<ResourceType>(slot.desc);
        }
        ResourceId<ResourceType> resourceId = slot.frameResources[index];
        // 从代码设计上，这里返回的id可能是无效id,但是如果没有在graph创建后再修改资源，不会出现无效id
        // 所以这里不检查，并认为返回的指针一定有效
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
        ResourceSlot<ResourceType> slot(std::forward(desc));
        slot.resourceCount=readOnly?1:Render::Config::MaxFramesInFlight;
        slots.push_back(slot);
        auto iter = std::prev(slots.end());
        slotMap[id] = iter;
    }
    template <typename ResourceType, typename ResourceDescType>
        requires IsResource<ResourceType>::value
    AccessId<ResourceType> CreateSlot(ResourceDescType&& desc)
    {
        return CreateSlot(std::forward<ResourceDescType>(desc),false);
    }
    template<typename ResourceType>
        requires IsResource<ResourceType>::value
    bool SetSlotReadOnly(AccessId<ResourceType> id)
    {
        auto& indexedSlots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto& slots = indexedSlots.m_Slots;
        auto& slotMap = indexedSlots.m_SlotMap;
        auto iter = slotMap.find(id);
        if (iter == slotMap.end())
        {
            assert(false && "Resource slot not found");
            return false;
        }
        ResourceSlot<ResourceType>& slot = *(*iter->second);
        // only need one resource for read only in multi in-flight frames
        slot.resourceCount = 1;
        return true;
    }


private:
    Borrow<ResourceArena> m_ResourceArena;
    Borrow<ResourceLruPool> m_ResourcePool;
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