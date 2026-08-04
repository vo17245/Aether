#pragma once
#include "ResourceId.h"
#include "ResourceTypeTraits.h"
#include "Render/RHI.h"
namespace Aether::RenderGraph
{
class ResourceArena
{
public:
    void AddDependency(ResourceId<rhi::TextureView> imageView, ResourceId<rhi::Texture2D> texture)
    {
        if (m_TextureToImageViewMap.find(texture) == m_TextureToImageViewMap.end())
        {
            m_TextureToImageViewMap[texture] = std::vector<ResourceId<rhi::TextureView>>();
        }
        m_TextureToImageViewMap[texture].push_back(imageView);
    }

    void DestroyImageView(ResourceId<rhi::TextureView> imageView)
    {
        // destroy image view
        auto imageViewIter = m_ImageViewMap.find(imageView);
        if (imageViewIter != m_ImageViewMap.end())
        {
            m_ImageViews.erase(imageViewIter->second);
            m_ImageViewMap.erase(imageViewIter);
        }
    }
    void DestroyTexture(ResourceId<rhi::Texture2D> texture)
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

    ResourceId<rhi::Texture2D> AddTexture(Scope<rhi::Texture2D>&& texture)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::Texture2D>();
        m_Textures.push_back(std::move(texture));
        auto iter = m_Textures.end();
        --iter;
        m_TextureMap[id] = iter;
        return id;
    }


    ResourceId<rhi::TextureView> AddImageView(Scope<rhi::TextureView>&& imageView)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::TextureView>();
        m_ImageViews.push_back(std::move(imageView));
        auto iter = m_ImageViews.end();
        --iter;
        m_ImageViewMap[id] = iter;
        return id;
    }
    ResourceId<rhi::VertexBuffer> AddVertexBuffer(Scope<rhi::VertexBuffer>&& buffer)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::VertexBuffer>();
        m_VertexBuffers.push_back(std::move(buffer));
        auto iter = m_VertexBuffers.end();
        --iter;
        m_VertexBufferMap[id] = iter;
        return id;
    }
    ResourceId<rhi::IndexBuffer> AddIndexBuffer(Scope<rhi::IndexBuffer>&& buffer)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::IndexBuffer>();
        m_IndexBuffers.push_back(std::move(buffer));
        auto iter = m_IndexBuffers.end();
        --iter;
        m_IndexBufferMap[id] = iter;
        return id;
    }
    ResourceId<rhi::UniformBuffer> AddUniformBuffer(Scope<rhi::UniformBuffer>&& buffer)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::UniformBuffer>();
        m_UniformBuffers.push_back(std::move(buffer));
        auto iter = m_UniformBuffers.end();
        --iter;
        m_UniformBufferMap[id] = iter;
        return id;
    }
    ResourceId<rhi::StagingBuffer> AddStagingBuffer(Scope<rhi::StagingBuffer>&& buffer)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::StagingBuffer>();
        m_StagingBuffers.push_back(std::move(buffer));
        auto iter = m_StagingBuffers.end();
        --iter;
        m_StagingBufferMap[id] = iter;
        return id;
    }
    ResourceId<rhi::RWStructuredBuffer> AddRWStructuredBuffer(Scope<rhi::RWStructuredBuffer>&& buffer)
    {
        auto id = m_ResourceIdAllocator.Allocate<rhi::RWStructuredBuffer>();
        m_RWStructuredBuffers.push_back(std::move(buffer));
        auto iter = m_RWStructuredBuffers.end();
        --iter;
        m_RWStructuredBufferMap[id] = iter;
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
        if constexpr (std::is_same_v<T, rhi::Texture2D>)
        {
            auto iter = m_TextureMap.find(id);
            if (iter != m_TextureMap.end())
            {
                return iter->second->Get();
            }
        }
        else if constexpr (std::is_same_v<T, rhi::TextureView>)
        {
            auto iter = m_ImageViewMap.find(id);
            if (iter != m_ImageViewMap.end())
            {
                return iter->second->Get();
            }
        }
        else if constexpr (std::is_same_v<T, rhi::VertexBuffer>)
        {
            auto iter = m_VertexBufferMap.find(id);
            if (iter != m_VertexBufferMap.end())
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

    template <typename T>
        requires IsResource<T>::value
    void Destroy(ResourceId<T> id)
    {
        if (!IsValid(id))
        {
            assert(false && "ResourceId is not valid");
            return;
        }
        if constexpr (std::is_same_v<T, rhi::Texture2D>)
        {
            DestroyTexture(id);
        }
        else if constexpr (std::is_same_v<T, rhi::TextureView>)
        {
            DestroyImageView(id);
        }
        else if constexpr (std::is_same_v<T, rhi::VertexBuffer>)
        {
            auto iter = m_VertexBufferMap.find(id);
            if (iter != m_VertexBufferMap.end())
            {
                m_VertexBuffers.erase(iter->second);
                m_VertexBufferMap.erase(iter);
            }
        }
        else if constexpr (std::is_same_v<T, rhi::IndexBuffer>)
        {
            auto iter = m_IndexBufferMap.find(id);
            if (iter != m_IndexBufferMap.end())
            {
                m_IndexBuffers.erase(iter->second);
                m_IndexBufferMap.erase(iter);
            }
        }
        else if constexpr (std::is_same_v<T, rhi::UniformBuffer>)
        {
            auto iter = m_UniformBufferMap.find(id);
            if (iter != m_UniformBufferMap.end())
            {
                m_UniformBuffers.erase(iter->second);
                m_UniformBufferMap.erase(iter);
            }
        }
        else if constexpr (std::is_same_v<T, rhi::StagingBuffer>)
        {
            auto iter = m_StagingBufferMap.find(id);
            if (iter != m_StagingBufferMap.end())
            {
                m_StagingBuffers.erase(iter->second);
                m_StagingBufferMap.erase(iter);
            }
        }
        else if constexpr (std::is_same_v<T, rhi::RWStructuredBuffer>)
        {
            auto iter = m_RWStructuredBufferMap.find(id);
            if (iter != m_RWStructuredBufferMap.end())
            {
                m_RWStructuredBuffers.erase(iter->second);
                m_RWStructuredBufferMap.erase(iter);
            }
        }
        else
        {
            // PrintType<T>();
            static_assert(always_false_v<T>, "Not implemented resource type");
        }
    }
    template <typename T>
    ResourceId<T> Import(T* resource)
    {
        auto id = m_ResourceIdAllocator.Allocate<T>();
        if constexpr (std::is_same_v<T, rhi::Texture2D>)
        {
            m_Textures.push_back(ResourceWrapper<T>(resource));
            auto iter = m_Textures.end();
            --iter;
            m_TextureMap[id] = iter;
        }
        else if constexpr (std::is_same_v<T, rhi::TextureView>)
        {
            m_ImageViews.push_back(ResourceWrapper<T>(resource));
            auto iter = m_ImageViews.end();
            --iter;
            m_ImageViewMap[id] = iter;
        }
        else if constexpr (std::is_same_v<T, rhi::VertexBuffer>)
        {
            m_VertexBuffers.push_back(ResourceWrapper<T>(resource));
            auto iter = m_VertexBuffers.end();
            --iter;
            m_VertexBufferMap[id] = iter;
        }
        else
        {
            static_assert(always_false_v<T>, "Not implemented resource type");
        }
        return id;
    }
    template <typename ResourceType>
    ResourceId<ResourceType> AddResource(Scope<ResourceType>&& resource)
    {
        if constexpr (std::is_same_v<ResourceType, rhi::Texture2D>)
        {
            return AddTexture(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, rhi::TextureView>)
        {
            return AddImageView(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, rhi::VertexBuffer>)
        {
            return AddVertexBuffer(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, rhi::IndexBuffer>)
        {
            return AddIndexBuffer(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, rhi::UniformBuffer>)
        {
            return AddUniformBuffer(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, rhi::StagingBuffer>)
        {
            return AddStagingBuffer(std::move(resource));
        }
        else if constexpr (std::is_same_v<ResourceType, rhi::RWStructuredBuffer>)
        {
            return AddRWStructuredBuffer(std::move(resource));
        }
        else
        {
            static_assert(always_false_v<ResourceType>, "Not implemented resource type");
        }
    }

private:
    template <typename T>
    struct ResourceWrapper
    {
        Scope<T> owned;
        T* imported = nullptr;
        T* Get()
        {
            if (imported)
            {
                return imported;
            }
            return owned.get();
        }
        ResourceWrapper(Scope<T>&& resource) : owned(std::move(resource))
        {
        }
        ResourceWrapper(T* resource) : imported(resource)
        {
        }
        ResourceWrapper() = default;
        ResourceWrapper(const ResourceWrapper&) = delete;
        ResourceWrapper(ResourceWrapper&&) = default;
        ResourceWrapper& operator=(const ResourceWrapper&) = delete;
        ResourceWrapper& operator=(ResourceWrapper&&) = default;
    };
    std::list<ResourceWrapper<rhi::TextureView>> m_ImageViews;
    std::unordered_map<ResourceId<rhi::TextureView>, typename std::list<ResourceWrapper<rhi::TextureView>>::iterator,
                       Hash<ResourceId<rhi::TextureView>>>
        m_ImageViewMap;

    std::list<ResourceWrapper<rhi::Texture2D>> m_Textures;
    std::unordered_map<ResourceId<rhi::Texture2D>, typename std::list<ResourceWrapper<rhi::Texture2D>>::iterator,
                       Hash<ResourceId<rhi::Texture2D>>>
        m_TextureMap;
    std::unordered_map<ResourceId<rhi::Texture2D>, std::vector<ResourceId<rhi::TextureView>>,
                       Hash<ResourceId<rhi::Texture2D>>>
        m_TextureToImageViewMap;
    ResourceIdAllocator m_ResourceIdAllocator;
    std::list<ResourceWrapper<rhi::VertexBuffer>> m_VertexBuffers;
    std::unordered_map<ResourceId<rhi::VertexBuffer>, typename std::list<ResourceWrapper<rhi::VertexBuffer>>::iterator,
                       Hash<ResourceId<rhi::VertexBuffer>>>
        m_VertexBufferMap;
    std::list<ResourceWrapper<rhi::IndexBuffer>> m_IndexBuffers;
    std::unordered_map<ResourceId<rhi::IndexBuffer>, typename std::list<ResourceWrapper<rhi::IndexBuffer>>::iterator,
                       Hash<ResourceId<rhi::IndexBuffer>>>
        m_IndexBufferMap;
    std::list<ResourceWrapper<rhi::UniformBuffer>> m_UniformBuffers;
    std::unordered_map<ResourceId<rhi::UniformBuffer>,
                       typename std::list<ResourceWrapper<rhi::UniformBuffer>>::iterator,
                       Hash<ResourceId<rhi::UniformBuffer>>>
        m_UniformBufferMap;
    std::list<ResourceWrapper<rhi::StagingBuffer>> m_StagingBuffers;
    std::unordered_map<ResourceId<rhi::StagingBuffer>,
                       typename std::list<ResourceWrapper<rhi::StagingBuffer>>::iterator,
                       Hash<ResourceId<rhi::StagingBuffer>>>
        m_StagingBufferMap;
    std::list<ResourceWrapper<rhi::RWStructuredBuffer>> m_RWStructuredBuffers;
    std::unordered_map<ResourceId<rhi::RWStructuredBuffer>,
                       typename std::list<ResourceWrapper<rhi::RWStructuredBuffer>>::iterator,
                       Hash<ResourceId<rhi::RWStructuredBuffer>>>
        m_RWStructuredBufferMap;
};

} // namespace Aether::RenderGraph