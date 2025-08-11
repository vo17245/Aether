#pragma once
#include "ResourceArena.h"
namespace Aether::RenderGraph
{

template<typename... Ts>
class ResourceLruPoolBase
{

private:// resource lru

    template<typename ResourceType>
    struct LruEntry
    {
        ResourceId<ResourceType> id;
        ResourceDescType<ResourceType> desc;
    };
    template<typename ResourceType>
    struct LruPool
    {
        using Id= ResourceId<ResourceType>;
        using List=std::list<LruEntry<ResourceType>>;
        using Key=ResourceDescType<ResourceType>;
        List resources;
        std::unordered_map<Key, std::vector<typename List::iterator>,Hash<Key>> resourceMap;
        Borrow<ResourceArena> arena;
        size_t capacity;
        LruPool(ResourceArena& _arena,size_t _capacity) :arena(_arena), capacity(_capacity) {}
        void Push(Key key,Id id)
        {
            auto iter=resourceMap.find(key);
            if(iter==resourceMap.end())
            {
                iter=resourceMap.insert({key,{}}).first;
            }
            std::vector<typename List::iterator>& vec=iter->second;
            resources.push_front({id,key});
            vec.push_back(resources.begin());
            if(resources.size() > capacity)
            {
                RemoveOldest();
            }
           
        }
        void RemoveOldest()
        {
            auto& oldest=resources.back();
            auto iter=resourceMap.find(oldest.desc);
            if(iter!=resourceMap.end())
            {
                auto& vec=iter->second;
                auto oldestIter = std::prev(resources.end());
                vec.erase(std::remove(vec.begin(), vec.end(), oldestIter), vec.end());
                if(vec.empty())
                {
                    resourceMap.erase(iter);
                }
            }
            arena->Destroy(oldest.id);
            resources.pop_back();
        }
        Id Pop(Key key)
        {
            auto iter=resourceMap.find(key);
            if(iter==resourceMap.end() || iter->second.empty())
            {
                return Id::CreateInvalid();
            }
            auto& vec=iter->second;
            auto lruIter=vec.back();
            vec.pop_back();
            if(vec.empty())
            {
                resourceMap.erase(iter);
            }
            Id id=lruIter->id;
            resources.erase(lruIter);
            return id;
        }

    };
    template<typename... Us>
    struct LruPoolListBase
    {
        std::tuple<LruPool<Us>...> pools;
    };
    using LruPoolList=LruPoolListBase<Ts...>;
    LruPoolList m_LruPoolList;
    
private://resource in use
    template <typename U>
    struct ResourceInUse
    {
        ResourceDescType<U>::Type desc;
        ResourceId<U> resourceId;
        uint32_t ttl;// time to live, in frames
    };
    template<typename... Us>
    struct ResourceInUseListBase
    {
        std::tuple<std::vector<ResourceInUse<Us>>...> resources;
    };
    using ResourceInUseList=ResourceInUseListBase<Ts...>;
    template<typename U>
    struct UpdateResourceInUse
    {
        LruPool<U>& pool;
        using ResourceType=U;
        using ResourceInUseArrayType=std::vector<ResourceInUse<U>>;
        void operator()(ResourceInUseArrayType& resourceInUse)
        {
            for(auto iter=resourceInUse.begin();iter!=resourceInUse.end();)
            {
                if(iter->ttl==0)
                {
                    pool.Push(iter->desc, iter->resourceId);
                    iter=resourceInUse.erase(iter);
                }
                else
                {
                    --iter->ttl;
                    ++iter;
                }
            }
        }
    };
    template<typename... Us>
    struct UpdateResourceInUseList;
    template<typename U>
    struct UpdateResourceInUseList<U>
    {
        ResourceInUseList& list;
        LruPoolList& lruPoolList;
        void operator()()
        {
            auto& resourceList = std::get<std::vector<ResourceInUse<U>>>(list.resources);
            auto& pool = std::get<LruPool<U>>(lruPoolList.pools);
            UpdateResourceInUse<U> {pool}();
        }
    };
    template<typename T,typename U,typename... Us>
    struct UpdateResourceInUseList<T,U,Us...>
    {
        ResourceInUseList& list;
        LruPoolList& lruPoolList;
        void operator()()
        {
            UpdateResourceInUseList<T>{list,lruPoolList}();
            UpdateResourceInUseList<U,Us...>{list,lruPoolList}();
        }
    };
    ResourceInUseList m_ResourceInUseList;
public:
    inline static constexpr size_t DefaultLruPoolSize=64;
    ResourceLruPoolBase(Borrow<ResourceArena> arena):
        m_LruPoolList{LruPool<Ts>(arena, DefaultLruPoolSize)...}
    {
    }
    template <typename T>
    void PushResourceInUse(ResourceId<T> resourceId,const ResourceDescType<T>::Type& desc)
    {
        auto& resourceList = std::get<std::vector<ResourceInUse<T>>>(m_ResourceInUseList.resources);
        resourceList.push_back({desc,resourceId, Render::Config::MaxFramesInFlight});
    }
    void OnFrameBegin()
    {
        // update resource in use
        UpdateResourceInUseList<Ts...>{m_ResourceInUseList, m_LruPoolList}();
    }
    template<typename ResourceType>
    ResourceId<ResourceType> Pop(const ResourceDescType<ResourceType>::Type& desc)
    {
        auto& pool = std::get<LruPool<ResourceType>>(m_LruPoolList.pools);
        return pool.Pop(desc);
    }
    template<typename ResourceType>
    void Push(const ResourceDescType<ResourceType>::Type& desc, ResourceId<ResourceType> id)
    {
        auto& pool = std::get<LruPool<ResourceType>>(m_LruPoolList.pools);
        pool.Push(desc, id);
    }
    template<typename ResourceType>
    void SetCapacity(size_t capacity)
    {
        auto& pool = std::get<LruPool<ResourceType>>(m_LruPoolList.pools);
        pool.capacity = capacity;
    }
    template <typename ResourceType>
    size_t GetCapacity() const
    {
        const auto& pool = std::get<LruPool<ResourceType>>(m_LruPoolList.pools);
        return pool.capacity;
    }

private:
    Borrow<ResourceArena> m_ResourceArena;
};
using ResourceLruPool = ResourceLruPoolBase<DeviceTexture, DeviceBuffer,  DeviceRenderPass, DeviceFrameBuffer,DeviceImageView>;
} // namespace Aether::RenderGraph