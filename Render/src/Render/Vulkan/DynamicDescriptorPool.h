#pragma once
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Render/Vulkan/DescriptorSet.h"
#include <optional>
#include <vector>
namespace Aether::vk
{
class DynamicDescriptorPool
{
public:
    struct PoolStorage
    {
        DescriptorPool pool;
        uint32_t freeUbo = 0;
        uint32_t freeSampler = 0;
        uint32_t freeSsbo = 0;
        uint32_t freeSets = 0;
        uint32_t maxUbo = 0;
        uint32_t maxSampler = 0;
        uint32_t maxSsbo = 0;
        uint32_t maxSets = 0;
        void Reset()
        {
            freeUbo = maxUbo;
            freeSampler = maxSampler;
            freeSsbo = maxSsbo;
            freeSets = maxSets;
            pool.ClearDescriptorSet();
        }
    };
    struct DescriptorResource
    {
        struct Accessor
        {
            uint32_t binding;
            uint32_t set;
        };
        std::vector<DescriptorSet> sets;
        std::vector<DescriptorSetLayout> layouts;
        // ubos[i] in sets[ubos[i].set] with binding ubos[i].binding
        std::vector<Accessor> ubos;
        std::vector<Accessor> samplers;
        std::vector<Accessor> ssbos;
        DescriptorResource() = default;
        DescriptorResource(DescriptorResource&&) = default;
        DescriptorResource& operator=(DescriptorResource&&) = default;
        DescriptorResource(const DescriptorResource&) = delete; 
        DescriptorResource& operator=(const DescriptorResource&) = delete;
    };
    /**
     * @brief 创建的同一个类型的descriptor都在同一个vulkan set中，并且ubo 在第一个set,sampler 在第二个set,ssbo在第三个set
     * 用于避免pipeline因为descriptor layout改变而重建,
     * @note 因为都创建在同一个set，所以只能创建在一个pool中，
     * 如果同一个descriptor 的数量超过一个pool的最大值时(默认值为1000，足够大，基本不会不够用)，会创建失败
    */
    std::optional<DescriptorResource> CreateSet(size_t uboCount, size_t samplerCount, size_t ssboCount)
    {
        DescriptorResource res;
        if(uboCount)
        {
            
            // check if need to add new pool
            if (m_UboPools.empty() || m_UboPools.back().freeUbo<uboCount )
            {
                if (!AddUboPool())
                {
                    return std::nullopt;
                }
            }
            // update info
            auto& uboPool = m_UboPools.back();
            uboPool.freeUbo -= uboCount;
            
            // create set layout and accessors
            DescriptorSetLayout::Builder builder;
            builder.SetBindingIndex(0);

            for (uint32_t i = 0; i < uboCount; i++)
            {
                builder.BeginUniformBinding()
                    .UseFragmentStage()
                    .UseVertexStage()
                    .EndUniformBinding();
                DescriptorResource::Accessor accessor;
                accessor.binding =  i;
                accessor.set = res.sets.size();
                res.ubos.push_back(accessor);
            }
            auto layout = builder.Build();
            if (!layout)
            {
                return std::nullopt;
            }
            // create set
            auto set = DescriptorSet::Create(layout.value(), uboPool.pool);

            res.sets.push_back(std::move(set.value()));
            res.layouts.push_back(std::move(layout.value()));
        }
        if(samplerCount)
        {
            // check if need to add new pool
            if (m_SamplerPools.empty() || m_SamplerPools.back().freeSampler <samplerCount)
            {
                if (!AddSamplerPool())
                {
                    return std::nullopt;
                }
            }
            // update info
            auto& samplerPool = m_SamplerPools.back();
            samplerPool.freeSampler -= samplerCount;
           
            // create set layout and accessors
            DescriptorSetLayout::Builder builder;
            builder.SetBindingIndex(0);
            for (uint32_t i = 0; i < samplerCount; i++)
            {
                builder.BeginSamplerBinding().UseFragmentStage().UseVertexStage().EndSamplerBinding();
                DescriptorResource::Accessor accessor;
                accessor.binding =  i;
                accessor.set = res.sets.size();
                res.samplers.push_back(accessor);
            }
            auto layout = builder.Build();
            if (!layout)
            {
                return std::nullopt;
            }
            // create set
            auto set = DescriptorSet::Create(layout.value(), samplerPool.pool);
            res.sets.push_back(std::move(set.value()));
            res.layouts.push_back(std::move(layout.value()));
        }
        if(ssboCount)
        {

            // check if need to add new pool
            if (m_SsboPools.empty() || m_SsboPools.back().freeSsbo <ssboCount)
            {
                if (!AddSsboPool())
                {
                    return std::nullopt;
                }
            }
            // update info
            auto& ssboPool = m_SsboPools.back();
            ssboPool.freeSsbo -= ssboCount;
           
            // create set layout and accessors
            DescriptorSetLayout::Builder builder;
            builder.SetBindingIndex(0);
            for (uint32_t i = 0; i < ssboCount; i++)
            {
                builder.BeginSSBO()
                    .UseFragmentStage()
                    .UseVertexStage()
                    .EndSSBO();
                DescriptorResource::Accessor accessor;
                accessor.binding =  i;
                accessor.set = res.sets.size();
                res.ssbos.push_back(accessor);
            }
            auto layout = builder.Build();
            if (!layout)
            {
                return std::nullopt;
            }
            // create set
            auto set = DescriptorSet::Create(layout.value(), ssboPool.pool);
            res.sets.push_back(std::move(set.value()));
            res.layouts.push_back(std::move(layout.value()));
        }
        return res;
    }
    /**
     * @brief 同一个类型的descriptor,可能出现在在两个vulkan set 中
     * @note 两次创建可能会获取不同的descriptor layout，这时需要重新创建pipeline，使用CreateSet函数创建来避免重建pipeline
    */
    std::optional<DescriptorResource> CreateSetDense(size_t uboCount, size_t samplerCount, size_t ssboCount)
    {
        DescriptorResource res;
        uint32_t uboIndex = 0;
        while (uboCount > 0)
        {
            // check if need to add new pool
            if (m_UboPools.empty() || m_UboPools.back().freeUbo==0 )
            {
                if (!AddUboPool())
                {
                    return std::nullopt;
                }
            }
            // update info
            auto& uboPool = m_UboPools.back();
            uint32_t curCount = 0;
            if (uboCount < uboPool.freeUbo)
            {
                curCount = uboCount;
                uboCount = 0;
                uboPool.freeUbo -= uboCount;
            }
            else
            {
                curCount = uboPool.freeUbo;
                uboCount -= uboPool.freeUbo;
                uboPool.freeUbo = 0;
            }
            // create set layout and accessors
            DescriptorSetLayout::Builder builder;
            builder.SetBindingIndex(uboIndex);

            for (uint32_t i = 0; i < curCount; i++)
            {
                builder.BeginUniformBinding()
                    .UseFragmentStage()
                    .UseVertexStage()
                    .EndUniformBinding();
                DescriptorResource::Accessor accessor;
                accessor.binding = uboIndex + i;
                accessor.set = res.sets.size();
                res.ubos.push_back(accessor);
            }
            auto layout = builder.Build();
            if (!layout)
            {
                return std::nullopt;
            }
            // create set
            auto set = DescriptorSet::Create(layout.value(), uboPool.pool);

            res.sets.push_back(std::move(set.value()));
            res.layouts.push_back(std::move(layout.value()));
        }
        while (samplerCount > 0)
        {
            // check if need to add new pool
            if (m_SamplerPools.empty() || m_SamplerPools.back().freeSampler == 0)
            {
                if (!AddSamplerPool())
                {
                    return std::nullopt;
                }
            }
            // update info
            auto& samplerPool = m_SamplerPools.back();
            uint32_t curCount = 0;
            if (samplerCount < samplerPool.freeSampler)
            {
                curCount = samplerCount;
                samplerCount = 0;
                samplerPool.freeSampler -= samplerCount;
            }
            else
            {
                curCount = samplerPool.freeSampler;
                samplerCount -= samplerPool.freeSampler;
                samplerPool.freeSampler = 0;
            }
            // create set layout and accessors
            DescriptorSetLayout::Builder builder;
            builder.SetBindingIndex(uboIndex);
            for (uint32_t i = 0; i < curCount; i++)
            {
                builder.BeginSamplerBinding().UseFragmentStage().UseVertexStage().EndSamplerBinding();
                DescriptorResource::Accessor accessor;
                accessor.binding = uboIndex + i;
                accessor.set = res.sets.size();
                res.samplers.push_back(accessor);
            }
            auto layout = builder.Build();
            if (!layout)
            {
                return std::nullopt;
            }
            // create set
            auto set = DescriptorSet::Create(layout.value(), samplerPool.pool);
            res.sets.push_back(std::move(set.value()));
            res.layouts.push_back(std::move(layout.value()));
        }
        while (ssboCount > 0)
        {
            // check if need to add new pool
            if (m_SsboPools.empty() || m_SsboPools.back().freeSsbo == 0)
            {
                if (!AddSsboPool())
                {
                    return std::nullopt;
                }
            }
            // update info
            auto& ssboPool = m_SsboPools.back();
            uint32_t curCount = 0;
            if (ssboCount < ssboPool.freeSsbo)
            {
                curCount = ssboCount;
                ssboCount = 0;
                ssboPool.freeSsbo -= ssboCount;
            }
            else
            {
                curCount = ssboPool.freeSsbo;
                ssboCount -= ssboPool.freeSsbo;
                ssboPool.freeSsbo = 0;
            }
            // create set layout and accessors
            DescriptorSetLayout::Builder builder;
            builder.SetBindingIndex(uboIndex);
            for (uint32_t i = 0; i < curCount; i++)
            {
                builder.BeginSSBO()
                    .UseFragmentStage()
                    .UseVertexStage()
                    .EndSSBO();
                DescriptorResource::Accessor accessor;
                accessor.binding = uboIndex + i;
                accessor.set = res.sets.size();
                res.ssbos.push_back(accessor);
            }
            auto layout = builder.Build();
            if (!layout)
            {
                return std::nullopt;
            }
            // create set
            auto set = DescriptorSet::Create(layout.value(), ssboPool.pool);
            res.sets.push_back(std::move(set.value()));
            res.layouts.push_back(std::move(layout.value()));
        }
        return res;
    }
    void Clear()
    {
        // reset pool
        for (auto& pool : m_UboPools)
        {
            pool.Reset();
        }
        for (auto& pool : m_SamplerPools)
        {
            pool.Reset();
        }
        for (auto& pool : m_SsboPools)
        {
            pool.Reset();
        }
        // move to cache
        for(auto& pool:m_UboPools)
        {
            m_UboPoolCache.push_back(std::move(pool));
        }
        for(auto& pool:m_SamplerPools)
        {
            m_SamplerPoolCache.push_back(std::move(pool));
        }
        for(auto& pool:m_SsboPools)
        {
            m_SsboPoolCache.push_back(std::move(pool));
        }
        // clear pool
        m_UboPools.clear();
        m_SamplerPools.clear();
        m_SsboPools.clear();
    }

private:
    bool AddUboPool()
    {
        // from cache?
        if (!m_UboPoolCache.empty())
        {
            m_UboPools.push_back(std::move(m_UboPoolCache.back()));
            m_UboPoolCache.pop_back();
            return true;
        }
        // create new
        DescriptorPool::Builder builder;
        builder.MaxSets(m_MaxSets);
        builder.PushUBO(m_UboPoolCapacity);
        auto pool = builder.Build();
        if (!pool)
        {
            return false;
        }
        m_UboPools.push_back({std::move(pool.value()),
                              m_UboPoolCapacity,
                              0, 0, 0,
                              m_UboPoolCapacity, 0, 0, m_MaxSets});
        return true;
    }
    bool AddSamplerPool()
    {
        // from cache?
        if (!m_SamplerPoolCache.empty())
        {
            m_SamplerPools.push_back(std::move(m_SamplerPoolCache.back()));
            m_SamplerPoolCache.pop_back();
            return true;
        }
        // create new
        DescriptorPool::Builder builder;
        builder.MaxSets(m_MaxSets);
        builder.PushSampler(m_SamplerPoolCapacity);
        auto pool = builder.Build();
        if (!pool)
        {
            return false;
        }
        m_SamplerPools.push_back({std::move(pool.value()),
                                  0,
                                  m_SamplerPoolCapacity, 0, 0,
                                  0, m_SamplerPoolCapacity, 0, m_MaxSets});
        return true;
    }
    bool AddSsboPool()
    {
        // from cache?
        if (!m_SsboPoolCache.empty())
        {
            m_SsboPools.push_back(std::move(m_SsboPoolCache.back()));
            m_SsboPoolCache.pop_back();
            return true;
        }
        // create new
        DescriptorPool::Builder builder;
        builder.MaxSets(m_MaxSets);
        builder.PushSSBO(m_SsboPoolCapacity);
        auto pool = builder.Build();
        if (!pool)
        {
            return false;
        }
        m_SsboPools.push_back({std::move(pool.value()),
                               0, 0,
                               m_SsboPoolCapacity, 0,
                               0, 0, m_SsboPoolCapacity, m_MaxSets});
        return true;
    }
    uint32_t m_UboPoolCapacity = 1000;
    uint32_t m_SamplerPoolCapacity = 1000;
    uint32_t m_SsboPoolCapacity = 1000;
    uint32_t m_MaxSets = 1000;
    // clear per frame
    std::vector<PoolStorage> m_UboPools;
    std::vector<PoolStorage> m_SamplerPools;
    std::vector<PoolStorage> m_SsboPools;
    // cache for reuse
    std::vector<PoolStorage> m_UboPoolCache;
    std::vector<PoolStorage> m_SamplerPoolCache;
    std::vector<PoolStorage> m_SsboPoolCache;
};
} // namespace Aether::vk