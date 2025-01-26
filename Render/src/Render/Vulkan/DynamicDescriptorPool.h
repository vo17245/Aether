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
    };
    std::optional<DescriptorResource> CreateSet(int uboCount, int samplerCount, int ssboCount)
    {
        DescriptorResource res;
        uint32_t uboIndex = 0;
        while (uboCount > 0)
        {
            // check if need to add new pool
            if (m_UboPools.empty() || m_UboPools.back().freeUbo == 0)
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

private:
    bool AddUboPool()
    {
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
    uint32_t m_UboPoolCapacity = 32;
    uint32_t m_SamplerPoolCapacity = 32;
    uint32_t m_SsboPoolCapacity = 32;
    uint32_t m_MaxSets = 32;
    std::vector<PoolStorage> m_UboPools;
    std::vector<PoolStorage> m_SamplerPools;
    std::vector<PoolStorage> m_SsboPools;
};
} // namespace Aether::vk