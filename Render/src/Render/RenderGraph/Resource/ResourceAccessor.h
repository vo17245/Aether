#pragma once
#include "AccessId.h"
#include "ResourceArena.h"
#include "DeviceTexture.h"
#include "ResourceLruPool.h"
#include "DeviceBuffer.h"
#include "../Utils.h"
#include "ResourceCode.h"
namespace Aether::RenderGraph
{

template <typename T>
struct VirtualResourceInfo
{
};
template <>
struct VirtualResourceInfo<DeviceTexture>
{
    DeviceImageLayout layout = DeviceImageLayout::Undefined;
};
template <typename T>
struct ResourceSlot
{
    template <typename U>
    ResourceSlot(U&& _desc) : desc(std::forward<U>(_desc))
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
    bool external = false;                                           // if the resource is created by the render graph
    AccessId<T> id = AccessId<T>{.handle = Handle::CreateInvalid()}; // access id for the resource
    VirtualResourceInfo<T> virtualInfo;                              // for image layout transition
};

template <typename... Ts>
class ResourceAccessorBase
{
public:
    void SetCurrentFrame(uint32_t frame)
    {
        m_FrameIndex=frame;
    }
    ResourceAccessorBase(Borrow<ResourceArena> resourceArena, Borrow<ResourceLruPool> resourcePool) :
        m_ResourceArena(resourceArena), m_ResourcePool(resourcePool), m_FrameIndex(0)
    {
    }
    ~ResourceAccessorBase()
    {
        ForEachInTuple(m_Slots, [&](auto&& indexedSlot) {
            auto& slots = indexedSlot.m_Slots;
            for (auto& slot : slots)
            {
                for (size_t i = 0; i < slot.resourceCount; ++i)
                {
                    if (slot.external)
                    {
                        continue;
                    }
                    if (slot.isRealized[i])
                    {
                        m_ResourcePool->PushResourceInUse(slot.frameResources[i], slot.desc);
                    }
                }
            }
        });
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
        uint32_t index = m_FrameIndex % slot.resourceCount;
        if (!slot.isRealized[index])
        {
            if constexpr (std::is_same_v<ResourceType,DeviceImageView> )
            {
                ResourceSlot<DeviceImageView>& imageViewSlot = slot;
                // create image view
                GetResource(imageViewSlot.desc.texture);// 确保texture已经realize
                auto resource=Realize<DeviceImageView>{*m_ResourceArena,*this,m_FrameIndex}(slot.desc);
                // add to arena
                if (!resource)
                {
                    assert(false && "Failed to create image view");
                    return nullptr;
                }
                auto resourceId=m_ResourceArena->AddResource(std::move(resource));
                
                // set resource dependency
                {
                    // get texture slot
                    ResourceSlot<DeviceTexture>& textureSlot = GetSlot(slot.desc.texture);
                    uint32_t textureIndex = m_FrameIndex % textureSlot.resourceCount;
                    assert(textureSlot.isRealized[textureIndex]);// 在realize image view时已经检查过
                    m_ResourceArena->AddDependency(resourceId, textureSlot.frameResources[textureIndex]);
                }
                // set slot resource id
                slot.frameResources[index] = resourceId;
                // set realized flag
                slot.isRealized[index] = true;
            }
            else if constexpr (std::is_same_v<ResourceType,DeviceFrameBuffer>)
            {
                // create frame buffer
                ResourceSlot<DeviceFrameBuffer>& frameBufferSlot = slot;
                auto resource=Realize<DeviceFrameBuffer>{*m_ResourceArena,*this,m_FrameIndex}(slot.desc);
                if(!resource)
                {
                    assert(false && "Failed to create frame buffer");
                    return nullptr;
                }
                // add to arena
                auto resourceId=m_ResourceArena->AddResource(std::move(resource));  


                // set resource dependency
                {
                    // set color attachments
                    for(size_t i=0;i<frameBufferSlot.desc.colorAttachmentCount;++i)
                    {
                        auto& attachment=frameBufferSlot.desc.colorAttachments[i];
                        ResourceSlot<DeviceImageView>& imageViewSlot = GetSlot(attachment.imageView);
                        uint32_t imageViewIndex = m_FrameIndex % imageViewSlot.resourceCount;
                        assert(imageViewSlot.isRealized[imageViewIndex]);// 在realize frame buffer时已经检查过
                        m_ResourceArena->AddDependency(resourceId, imageViewSlot.frameResources[imageViewIndex]);

                    }
                    // set depth attachment
                    if (frameBufferSlot.desc.depthAttachment)
                    {
                        auto& attachment = frameBufferSlot.desc.depthAttachment.value();
                        ResourceSlot<DeviceImageView>& imageViewSlot = GetSlot(attachment.imageView);
                        uint32_t imageViewIndex = m_FrameIndex % imageViewSlot.resourceCount;
                        assert(imageViewSlot.isRealized[imageViewIndex]);// 在realize frame buffer时已经检查过
                        m_ResourceArena->AddDependency(resourceId, imageViewSlot.frameResources[imageViewIndex]);
                    }

                }
                // set slot resource id
                slot.frameResources[index] = resourceId;
                // set realized flag
                slot.isRealized[index] = true;
            }
            else
            {
                // create resource
                auto resource = Realize<ResourceType>{}(slot.desc);
                if (!resource)
                {
                    assert(false && "Failed to create resource");
                    return nullptr;
                }
                // add to arena
                auto resourceId = m_ResourceArena->AddResource(std::move(resource));
                // set slot resource id
                slot.frameResources[index] = resourceId;
                // set realized flag
                slot.isRealized[index] = true;
            }
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
        slots.m_Slots.erase((*iter_to).second);
        iter_to->second = iter_from->second;
    }

    template <typename ResourceType, typename ResourceDescType>
    ResourceSlot<ResourceType>& CreateSlot(ResourceDescType&& desc)
    {
        auto& indexedSlots = std::get<IndexedSlot<ResourceType>>(m_Slots);
        auto& slots = indexedSlots.m_Slots;
        auto& slotMap = indexedSlots.m_SlotMap;
        AccessId<ResourceType> id = m_Allocator.Allocate<ResourceType>();
        ResourceSlot<ResourceType> slot(std::forward<ResourceDescType>(desc));
        slot.resourceCount = 1;
        slot.id = id;
        slot.desc = desc;
        slots.push_back(std::move(slot));
        auto iter = std::prev(slots.end());
        slotMap[id] = iter;
        SetSlotVirtualInfo<ResourceType>(desc, iter->virtualInfo);
        return slots.back();
    }
    template <typename ResourceType>
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
        ResourceSlot<ResourceType>& slot = *((*iter).second);
        if(slot.external)
        {
            return false;
        }
        // only need one resource for read only in multi in-flight frames
        slot.resourceCount = Render::Config::MaxFramesInFlight;
        return true;
    }
    template <typename ResourceType>
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

private:
    template <typename ResourceType>
    void SetSlotVirtualInfo(const typename ResourceDescType<ResourceType>::Type& desc,
                            VirtualResourceInfo<ResourceType>& info)
    {
    }
    template <>
    void SetSlotVirtualInfo<DeviceTexture>(const typename ResourceDescType<DeviceTexture>::Type& desc,
                                           VirtualResourceInfo<DeviceTexture>& info)
    {
        info.layout = desc.layout;
    }

private:
    Borrow<ResourceArena> m_ResourceArena;
    Borrow<ResourceLruPool> m_ResourcePool;
    uint32_t m_FrameIndex; // current frame index
    template <typename ResourceType>
    struct IndexedSlot
    {
        std::list<ResourceSlot<ResourceType>> m_Slots;
        std::unordered_map<AccessId<ResourceType>, typename std::list<ResourceSlot<ResourceType>>::iterator,
                           Hash<AccessId<ResourceType>>>
            m_SlotMap;
    };
    std::tuple<IndexedSlot<Ts>...> m_Slots;
    AccessIdAllocator m_Allocator;
};
namespace Detail
{
template <typename T>
struct BuildResourceAccessor;
template <typename... Ts>
struct BuildResourceAccessor<TypeArray<Ts...>>
{
    using Type = ResourceAccessorBase<Ts...>;
};
} // namespace Detail
using ResourceAccessor = typename Detail::BuildResourceAccessor<ResourceTypeArray>::Type;

template <>
struct Realize<DeviceImageView>
{
    ResourceArena& arena;
    ResourceAccessor& accessor;
    uint32_t frameIndex;
    Scope<DeviceImageView> operator()(const ImageViewDesc& desc)
    {
        auto& textureSlot = accessor.GetSlot(desc.texture);
        uint32_t textureIndex = frameIndex % textureSlot.resourceCount;
        assert(textureSlot.isRealized[textureIndex]);
        auto& textureId = textureSlot.frameResources[textureIndex];
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
        DeviceRenderPass* renderPass = accessor.GetResource(desc.renderPass);
        if (!renderPass || renderPass->Empty())
        {
            return nullptr;
        }
        frameBufferDesc.width = desc.width;
        frameBufferDesc.height = desc.height;
        frameBufferDesc.colorAttachmentCount = desc.colorAttachmentCount;
        for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
        {
            auto& attachment = desc.colorAttachments[i];
            auto& imageViewSlot = accessor.GetSlot(attachment.imageView);
            uint32_t imageViewIndex = frameIndex % imageViewSlot.resourceCount;
            if(!imageViewSlot.isRealized[imageViewIndex])
            {
                accessor.GetResource(attachment.imageView);
            }
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
            if(!imageViewSlot.isRealized[imageViewIndex])
            {
                accessor.GetResource(attachment.imageView);
            }
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