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
                return iter->second->Get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceImageView>)
        {
            auto iter = m_ImageViewMap.find(id);
            if (iter != m_ImageViewMap.end())
            {
                return iter->second->Get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceFrameBuffer>)
        {
            auto iter = m_FrameBufferMap.find(id);
            if (iter != m_FrameBufferMap.end())
            {
                return iter->second->Get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceRenderPass>)
        {
            auto iter = m_RenderPassMap.find(id);
            if (iter != m_RenderPassMap.end())
            {
                return iter->second->Get();
            }
        }
        else if constexpr (std::is_same_v<T, DeviceBuffer>)
        {
            auto iter = m_BufferMap.find(id);
            if (iter != m_BufferMap.end())
            {
                return iter->second->Get();
            }
        }
        else
        {
            static_assert(always_false_v<T>, "Not implemented resource type");
        }
        return nullptr;
    }

    template<typename T>
    requires IsResource<T>::value
    void Destroy(ResourceId<T> id)
    {
        if (!IsValid(id))
        {
            assert(false && "ResourceId is not valid");
            return;
        }
        if constexpr (std::is_same_v<T, DeviceTexture>)
        {
            DestroyTexture(id);
        }
        else if constexpr (std::is_same_v<T, DeviceImageView>)
        {
            DestroyImageView(id);
        }
        else if constexpr (std::is_same_v<T, DeviceFrameBuffer>)
        {
            DestroyFrameBuffer(id);
        }
        else if constexpr (std::is_same_v<T, DeviceRenderPass>)
        {
            auto iter = m_RenderPassMap.find(id);
            if (iter != m_RenderPassMap.end())
            {
                m_RenderPasses.erase(iter->second);
                m_RenderPassMap.erase(iter);
            }
        }
        else if constexpr (std::is_same_v<T, DeviceBuffer>)
        {
            auto iter = m_BufferMap.find(id);
            if (iter != m_BufferMap.end())
            {
                m_Buffers.erase(iter->second);
                m_BufferMap.erase(iter);
            }
        }
        else
        {
            //PrintType<T>();
            static_assert(always_false_v<T>, "Not implemented resource type");
        }
    }
    template<typename T>
    ResourceId<T> Import(T* resource)
    {
        auto id = m_ResourceIdAllocator.Allocate<T>();
        if constexpr (std::is_same_v<T, DeviceTexture>)
        {
            m_Textures.push_back(ResourceWrapper<T>(resource));
            auto iter = m_Textures.end();
            --iter;
            m_TextureMap[id] = iter;
        }
        else if constexpr (std::is_same_v<T, DeviceImageView>)
        {
            m_ImageViews.push_back(ResourceWrapper<T>(resource));
            auto iter = m_ImageViews.end();
            --iter;
            m_ImageViewMap[id] = iter;
        }
        else if constexpr (std::is_same_v<T, DeviceFrameBuffer>)
        {
            m_FrameBuffers.push_back(ResourceWrapper<T>(resource));
            auto iter = m_FrameBuffers.end();
            --iter;
            m_FrameBufferMap[id] = iter;
        }
        else if constexpr (std::is_same_v<T, DeviceRenderPass>)
        {
            m_RenderPasses.push_back(ResourceWrapper<T>(resource));
            auto iter = m_RenderPasses.end();
            --iter;
            m_RenderPassMap[id] = iter;
        }
        else if constexpr (std::is_same_v<T, DeviceBuffer>)
        {
            m_Buffers.push_back(ResourceWrapper<T>(resource));
            auto iter = m_Buffers.end();
            --iter;
            m_BufferMap[id] = iter;
        }
        else
        {
            static_assert(always_false_v<T>, "Not implemented resource type");
        }
        return id;
    }
    template<typename ResourceType>
    ResourceId<ResourceType> AddResource(Scope<ResourceType>&& resource)
    {
        if constexpr (std::is_same_v<ResourceType, DeviceTexture>)
        {
            return AddTexture(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, DeviceImageView>)
        {
            return AddImageView(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, DeviceFrameBuffer>)
        {
            return AddFrameBuffer(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, DeviceRenderPass>)
        {
            auto id = m_ResourceIdAllocator.Allocate<DeviceRenderPass>();
            m_RenderPasses.push_back(std::move(resource));
            auto iter = m_RenderPasses.end();
            --iter;
            m_RenderPassMap[id] = iter;
            return id;
        }
        else if constexpr (std::is_same_v<ResourceType, DeviceBuffer>)
        {
            auto id = m_ResourceIdAllocator.Allocate<DeviceBuffer>();
            m_Buffers.push_back(std::move(resource));
            auto iter = m_Buffers.end();
            --iter;
            m_BufferMap[id] = iter;
            return id;
        }
        else
        {
            static_assert(always_false_v<ResourceType>, "Not implemented resource type");
        }
    }
private:
    template<typename T>
    struct ResourceWrapper
    {
        Scope<T> owned;
        T* imported=nullptr;
        T* Get()
        {
            if (imported)
            {
                return imported;
            }
            return owned.get();
        }
        ResourceWrapper(Scope<T>&& resource) : owned(std::move(resource)) {}
        ResourceWrapper(T* resource) : imported(resource) {}
        ResourceWrapper() = default;
        ResourceWrapper(const ResourceWrapper&) = delete;
        ResourceWrapper(ResourceWrapper&&) = default;
        ResourceWrapper& operator=(const ResourceWrapper&) = delete;
        ResourceWrapper& operator=(ResourceWrapper&&) = default;
    };
    std::list<ResourceWrapper<DeviceFrameBuffer>> m_FrameBuffers;
    std::unordered_map<ResourceId<DeviceFrameBuffer>, typename std::list<ResourceWrapper<DeviceFrameBuffer>>::iterator, Hash<ResourceId<DeviceFrameBuffer>>> m_FrameBufferMap;
    std::list<ResourceWrapper<DeviceImageView>> m_ImageViews;
    std::unordered_map<ResourceId<DeviceImageView>, typename std::list<ResourceWrapper<DeviceImageView>>::iterator, Hash<ResourceId<DeviceImageView>>> m_ImageViewMap;

    std::unordered_map<ResourceId<DeviceImageView>, std::vector<ResourceId<DeviceFrameBuffer>>, Hash<ResourceId<DeviceImageView>>> m_ImageViewToFrameBufferMap;
    std::list<ResourceWrapper<DeviceTexture>> m_Textures;
    std::unordered_map<ResourceId<DeviceTexture>, typename std::list<ResourceWrapper<DeviceTexture>>::iterator, Hash<ResourceId<DeviceTexture>>> m_TextureMap;
    std::unordered_map<ResourceId<DeviceTexture>, std::vector<ResourceId<DeviceImageView>>, Hash<ResourceId<DeviceTexture>>> m_TextureToImageViewMap;
    ResourceIdAllocator m_ResourceIdAllocator;
    std::list<ResourceWrapper<DeviceRenderPass>> m_RenderPasses;
    std::unordered_map<ResourceId<DeviceRenderPass>, typename std::list<ResourceWrapper<DeviceRenderPass>>::iterator, Hash<ResourceId<DeviceRenderPass>>> m_RenderPassMap;
    std::list<ResourceWrapper<DeviceBuffer>> m_Buffers;
    std::unordered_map<ResourceId<DeviceBuffer>, typename std::list<ResourceWrapper<DeviceBuffer>>::iterator, Hash<ResourceId<DeviceBuffer>>> m_BufferMap;
};




} // namespace Aether::RenderGraph