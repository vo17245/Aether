#pragma once
#include <Render/RenderApi.h>
#include "ResourceId.h"
#include "ResourceTypeTraits.h"
#include "DeviceFrameBuffer.h"
#include "DeviceRenderPass.h"
#include "DeviceImageView.h"
#include "DeviceTexture.h"
namespace Aether::RenderGraph
{
class ResourceArena
{
public:
    void AddDependency(ResourceId<DeviceImageView> imageView, ResourceId<DeviceTexture> texture)
    {
        if (m_TextureToImageViewMap.find(texture) == m_TextureToImageViewMap.end())
        {
            m_TextureToImageViewMap[texture] = std::vector<ResourceId<DeviceImageView>>();
        }
        m_TextureToImageViewMap[texture].push_back(imageView);
    }
    void AddDependency(ResourceId<DeviceFrameBuffer> frameBuffer, ResourceId<DeviceImageView> imageView)
    {
        if (m_ImageViewToFrameBufferMap.find(imageView) == m_ImageViewToFrameBufferMap.end())
        {
            m_ImageViewToFrameBufferMap[imageView] = std::vector<ResourceId<DeviceFrameBuffer>>();
        }
        m_ImageViewToFrameBufferMap[imageView].push_back(frameBuffer);
    }
    void DestroyImageView(ResourceId<DeviceImageView> imageView)
    {
        // destroy frame buffers that depend on this image view
        auto iter = m_ImageViewToFrameBufferMap.find(imageView);
        if (iter != m_ImageViewToFrameBufferMap.end())
        {
            for (auto frameBuffer : iter->second)
            {
                auto fbIter = m_FrameBufferMap.find(frameBuffer);
                if (fbIter != m_FrameBufferMap.end())
                {
                    m_FrameBuffers.erase(fbIter->second);
                    m_FrameBufferMap.erase(fbIter);
                }
            }
            m_ImageViewToFrameBufferMap.erase(iter);
        }
        // destroy image view
        auto imageViewIter = m_ImageViewMap.find(imageView);
        if (imageViewIter != m_ImageViewMap.end())
        {
            m_ImageViews.erase(imageViewIter->second);
            m_ImageViewMap.erase(imageViewIter);
        }
    }
    void DestroyTexture(ResourceId<DeviceTexture> texture)
    {
        // destroy image views that depend on this texture
        auto iter = m_TextureToImageViewMap.find(texture);
        if (iter != m_TextureToImageViewMap.end())
        {
            for (auto imageView : iter->second)
            {
                DestroyImageView(imageView);
            }
            m_TextureToImageViewMap.erase(iter);
        }
        // destroy texture
        auto textureIter = m_TextureMap.find(texture);
        if (textureIter != m_TextureMap.end())
        {
            m_Textures.erase(textureIter->second);
            m_TextureMap.erase(textureIter);
        }
    }
    void DestroyFrameBuffer(ResourceId<DeviceFrameBuffer> frameBuffer)
    {
        // destroy frame buffer
        auto iter = m_FrameBufferMap.find(frameBuffer);
        if (iter != m_FrameBufferMap.end())
        {
            m_FrameBuffers.erase(iter->second);
            m_FrameBufferMap.erase(iter);
        }
    }
    ResourceId<DeviceFrameBuffer> AddFrameBuffer(Scope<DeviceFrameBuffer>&& frameBuffer)
    {
        auto id = m_ResourceIdAllocator.Allocate<DeviceFrameBuffer>();
        m_FrameBuffers.push_back(std::move(frameBuffer));
        auto iter = m_FrameBuffers.end();
        --iter;
        m_FrameBufferMap[id] = iter;
        return id;
    }
    ResourceId<DeviceImageView> AddImageView(Scope<DeviceImageView>&& imageView)
    {
        auto id = m_ResourceIdAllocator.Allocate<DeviceImageView>();
        m_ImageViews.push_back(std::move(imageView));
        auto iter = m_ImageViews.end();
        --iter;
        m_ImageViewMap[id] = iter;
        return id;
    }
    ResourceId<DeviceTexture> AddTexture(Scope<DeviceTexture>&& texture)
    {
        auto id = m_ResourceIdAllocator.Allocate<DeviceTexture>();
        m_Textures.push_back(std::move(texture));
        auto iter = m_Textures.end();
        --iter;
        m_TextureMap[id] = iter;
        return id;
    }

    template <typename T>
    bool IsValid(ResourceId<T> id)
    {
        return (m_ResourceIdAllocator.GetNextVersion(id) - 1) == id.handle.version;
    }
    template <typename T>
        requires IsResource<T>::value
    T* GetResource(ResourceId<T> id)
    {
        if (!IsValid(id))
        {
            assert(false && "ResourceId is not valid");
            return nullptr;
        }
        if constexpr (std::is_same_v<T, DeviceTexture>)
        {
            auto iter = m_TextureMap.find(id);
            if (iter != m_TextureMap.end())
            {
                return iter->second->get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceImageView>)
        {
            auto iter = m_ImageViewMap.find(id);
            if (iter != m_ImageViewMap.end())
            {
                return iter->second->get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceFrameBuffer>)
        {
            auto iter = m_FrameBufferMap.find(id);
            if (iter != m_FrameBufferMap.end())
            {
                return iter->second->get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceRenderPass>)
        {
            auto iter = m_RenderPassMap.find(id);
            if (iter != m_RenderPassMap.end())
            {
                return iter->second->get();
            }
        }
        else
        {
            static_assert(false, "Not implemented resource type");
        }
        return nullptr;
    }
    template <typename T>
        requires IsResource<T>::value
    ResourceId<T> Allocate(const ResourceDescType<T>::Type& desc);
    template<typename T>
    requires IsResource<T>::value
    void Destroy(ResourceId<T> id);

private:
    std::list<Scope<DeviceFrameBuffer>> m_FrameBuffers;
    std::unordered_map<ResourceId<DeviceFrameBuffer>, typename std::list<Scope<DeviceFrameBuffer>>::iterator, Hash<ResourceId<DeviceFrameBuffer>>> m_FrameBufferMap;
    std::list<Scope<DeviceImageView>> m_ImageViews;
    std::unordered_map<ResourceId<DeviceImageView>, typename std::list<Scope<DeviceImageView>>::iterator, Hash<ResourceId<DeviceImageView>>> m_ImageViewMap;

    std::unordered_map<ResourceId<DeviceImageView>, std::vector<ResourceId<DeviceFrameBuffer>>, Hash<ResourceId<DeviceImageView>>> m_ImageViewToFrameBufferMap;
    std::list<Scope<DeviceTexture>> m_Textures;
    std::unordered_map<ResourceId<DeviceTexture>, typename std::list<Scope<DeviceTexture>>::iterator, Hash<ResourceId<DeviceTexture>>> m_TextureMap;
    std::unordered_map<ResourceId<DeviceTexture>, std::vector<ResourceId<DeviceImageView>>, Hash<ResourceId<DeviceTexture>>> m_TextureToImageViewMap;
    ResourceIdAllocator m_ResourceIdAllocator;
    std::list<Scope<DeviceRenderPass>> m_RenderPasses;
    std::unordered_map<ResourceId<DeviceRenderPass>, typename std::list<Scope<DeviceRenderPass>>::iterator, Hash<ResourceId<DeviceRenderPass>>> m_RenderPassMap;
};
template <>
struct Realize<DeviceImageView>
{
    ResourceArena& arena;
    Scope<DeviceImageView> operator()(const ImageViewDesc& desc)
    {
        DeviceTexture* texture = arena.GetResource(desc.texture);
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
            auto& imageViewId = attachment.imageView;
            DeviceImageView* imageView = arena.GetResource(imageViewId);

            frameBufferDesc.colorAttachments[i] = imageView;
        }
        if (desc.depthAttachment)
        {
            auto& attachment = desc.depthAttachment.value();
            auto& imageViewId = attachment.imageView;
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
namespace Detail
{
template <typename T>
    requires IsResource<T>::value
struct RealizeAdapter
{
    ResourceArena& arena;
    Scope<T> operator()(const ResourceDescType<T>::Type& desc)
    {
        if (std::is_constructible_v<Realize<T>, ResourceArena&>)
        {
            return Realize<T>{arena}(desc);
        }
        else
        {
            return Realize<T>()(desc);
        }
    }
};
} // namespace Detail
template <typename T>
    requires IsResource<T>::value
inline ResourceId<T> ResourceArena::Allocate(const ResourceDescType<T>::Type& desc)
{
    // create resource
    Detail::RealizeAdapter<T> realize{*this};
    auto resource = realize(desc);
    if (!resource)
    {
        assert(false && "Failed to create resource");
        return ResourceId<T>::Invalid();
    }
    // allocate id
    auto id = m_ResourceIdAllocator.Allocate<T>();
    // push resource to list and set map
    if constexpr (std::is_same_v<T, DeviceTexture>)
    {
        m_Textures.push_back(std::move(resource));
        auto iter = m_Textures.end();
        --iter;
        m_TextureMap[id] = iter;
    }
    else if constexpr (std::is_same_v<T, DeviceImageView>)
    {
        m_ImageViews.push_back(std::move(resource));
        auto iter = m_ImageViews.end();
        --iter;
        m_ImageViewMap[id] = iter;
    }
    else if constexpr (std::is_same_v<T, DeviceFrameBuffer>)
    {
        m_FrameBuffers.push_back(std::move(resource));
        auto iter = m_FrameBuffers.end();
        --iter;
        m_FrameBufferMap[id] = iter;
    }
    else if constexpr (std::is_same_v<T, DeviceRenderPass>)
    {
        m_RenderPasses.push_back(std::move(resource));
        auto iter = m_RenderPasses.end();
        --iter;
        m_RenderPassMap[id] = iter;
    }
    else
    {
        static_assert(false, "Not implemented resource type");
    }
}

} // namespace Aether::RenderGraph