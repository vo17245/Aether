#pragma once
#include "AccessId.h"
#include "ResourceArena.h"
#include "DeviceTexture.h"
#include "ResourceLruPool.h"
#include "DeviceBuffer.h"
#include "../Utils.h"
namespace Aether::RenderGraph
{

template <typename T>
struct ResourceSlot
{
    template <typename U>
    ResourceSlot(U&& _desc) :
        desc(std::forward<U>(_desc))
    {
    }
    ResourceSlot() = default;
    ResourceSlot(const ResourceSlot&) = default;
    ResourceSlot(ResourceSlot&&) = default;
    ResourceSlot& operator=(const ResourceSlot&) = default;
    ResourceSlot& operator=(ResourceSlot&&) = default;

    ResourceDescType<T>::Type desc;
    ResourceId<T> frameResources[Render::Config::InFlightFrameResourceSlots];
    uint32_t resourceCount = 1; 
    bool isRealized[Render::Config::InFlightFrameResourceSlots] = {false}; // if the resource is realized
    bool external = false; // if the resource is created by the render graph
    AccessId<T> id=AccessId<T>{.handle=Handle::CreateInvalid()}; // access id for the resource
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
    ResourceType* GetResource(AccessId<ResourceType> id)
    {
        auto& slots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto iter = slots.m_SlotMap.find(id);
        if (iter == slots.m_SlotMap.end())
        {
            assert(false && "Resource slot not found");
            return nullptr;
        }
        ResourceSlot<ResourceType>& slot = *iter->second;
        uint32_t index=m_FrameIndex % slot.resourceCount;
        if(!slot.isRealized[index])
        {
            //slot.frameResources[index]=m_ResourcePool->PopOrCreate<ResourceType>(slot.desc);
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
    ResourceSlot<ResourceType>& CreateSlot(ResourceDescType&& desc)
    {
        auto& indexedSlots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto& slots = indexedSlots.m_Slots;
        auto& slotMap = indexedSlots.m_SlotMap;
        AccessId<ResourceType> id = m_Allocator.Allocate<ResourceType>();
        ResourceSlot<ResourceType> slot(std::forward<ResourceDescType>(desc));
        slot.resourceCount=1;
        slot.id = id;
        slots.push_back(std::move(slot));
        auto iter = std::prev(slots.end());
        slotMap[id] = iter;
        return slots.back();
    }
    template<typename ResourceType>
    bool SetSlotSupportsInFlightResources(AccessId<ResourceType> id)
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
        slot.resourceCount = Render::Config::MaxFramesInFlight;
        return true;
    }
    template<typename ResourceType>
    ResourceSlot<ResourceType>& GetSlot(AccessId<ResourceType> id)
    {
        auto& indexedSlots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto& slotMap = indexedSlots.m_SlotMap;
        auto iter = slotMap.find(id);
        if (iter == slotMap.end())
        {
            assert(false && "Resource slot not found");
        }
        return (*iter->second);
    }
    template<typename ResourceType,typename... ResourceIds>
    ResourceSlot<ResourceType>& CreateExternalSlot(const ResourceIds&... ids)
    {
        auto& indexedSlots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto& slots = indexedSlots.m_Slots;
        auto& slotMap = indexedSlots.m_SlotMap;
        AccessId<ResourceType> id = m_Allocator.Allocate<ResourceType>();
        ResourceSlot<ResourceType> slot;
        slot.id = id;
        slot.external = true;
        slot.resourceCount = sizeof...(ResourceIds);
        SetArray(slot.frameResources, ids...);
        slots.push_back(std::move(slot));
        auto iter = std::prev(slots.end());
        slotMap[id] = iter;
        return slots.back();
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




template <>
struct Realize<DeviceImageView>
{
    ResourceArena& arena;
    ResourceAccessor& accessor;
    uint32_t frameIndex;
    Scope<DeviceImageView> operator()(const ImageViewDesc& desc)
    {
        auto& textureSlot=accessor.GetSlot(desc.texture);
        uint32_t textureIndex=frameIndex%textureSlot.resourceCount;
        assert(textureSlot.isRealized[textureIndex]);
        auto& textureId=textureSlot.frameResources[textureIndex];
        assert(textureId.handle.IsValid());
        DeviceTexture* texture = arena.GetResource(textureId);
        if (!texture || texture->Empty())
        {
            return nullptr;
        }
        auto imageView = texture->CreateImageView(desc.desc);
        if (!imageView)
        {
            return nullptr;
        }
        return CreateScope<DeviceImageView>(std::move(imageView));
    }
};
template <>
struct Realize<DeviceFrameBuffer>
{
    ResourceArena& arena;
    ResourceAccessor& accessor;
    uint32_t frameIndex;
    Scope<DeviceFrameBuffer> operator()(const FrameBufferDesc& desc)
    {
        DeviceFrameBufferDesc frameBufferDesc;
        DeviceRenderPass* renderPass = arena.GetResource(desc.renderPass);
        if (!renderPass || renderPass->Empty())
        {
            return nullptr;
        }
        frameBufferDesc.width = desc.width;
        frameBufferDesc.height = desc.height;
        frameBufferDesc.colorAttachmentCount = desc.colorAttachmentCount;
        for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            auto& attachment = desc.colorAttachment[i];
            auto& imageViewSlot=accessor.GetSlot(attachment.imageView);
            uint32_t imageViewIndex = frameIndex % imageViewSlot.resourceCount;
            assert(imageViewSlot.isRealized[imageViewIndex]);
            assert(imageViewSlot.frameResources[imageViewIndex].handle.IsValid());
            ResourceId<DeviceImageView> imageViewId = imageViewSlot.frameResources[imageViewIndex];
            DeviceImageView* imageView = arena.GetResource(imageViewId);

            frameBufferDesc.colorAttachments[i] = imageView;
        }
        if (desc.depthAttachment)
        {
            auto& attachment = desc.depthAttachment.value();
            auto& imageViewSlot = accessor.GetSlot(attachment.imageView);
            uint32_t imageViewIndex = frameIndex % imageViewSlot.resourceCount;
            assert(imageViewSlot.isRealized[imageViewIndex]);
            assert(imageViewSlot.frameResources[imageViewIndex].handle.IsValid());
            ResourceId<DeviceImageView> imageViewId = imageViewSlot.frameResources[imageViewIndex];
            DeviceImageView* imageView = arena.GetResource(imageViewId);
            frameBufferDesc.depthAttachment = imageView;
        }
        auto frameBuffer = DeviceFrameBuffer::Create(*renderPass, frameBufferDesc);
        if (frameBuffer.Empty())
        {
            return nullptr;
        }
        return CreateScope<DeviceFrameBuffer>(std::move(frameBuffer));
    }
};









} // namespace Aether::RenderGraph