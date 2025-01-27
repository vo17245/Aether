#pragma once
#include <expected>
#include <variant>
#include "../Vulkan/DynamicDescriptorPool.h"
#include "Render/Config.h"
namespace Aether
{
using DeviceDescriptorSet=std::variant<std::monostate,vk::DynamicDescriptorPool::DescriptorResource>;

class DeviceDescriptorPool
{
public:
    DeviceDescriptorSet CreateSet(size_t ubo,size_t ssbo,size_t sampler)
    {
        return std::visit(CreateSetImpl{ubo,ssbo,sampler},m_DescriptorPool);
    }
    static std::expected<DeviceDescriptorPool, std::string> Create()
    {
        assert(Render::Config::RenderApi==Render::Api::Vulkan && "only support vulkan now");
        DeviceDescriptorPool pool;
        pool.m_DescriptorPool=vk::DynamicDescriptorPool();
        return pool;
    }
    operator bool() const
    {
        return m_DescriptorPool.index()!=0;
    }
private:
    struct CreateSetImpl
    {
        size_t ubo;
        size_t ssbo;
        size_t sampler;
        template<typename T>
        DeviceDescriptorSet operator()(T& pool)
        {
            static_assert(sizeof(T)==0,"not support" );
        }
        template<>
        DeviceDescriptorSet operator()(vk::DynamicDescriptorPool& pool)
        {
            auto set=pool.CreateSet(ubo, sampler, ssbo);
            if(!set)
            {
                return {};
            }
            return std::move(set.value());
        }
        template<>
        DeviceDescriptorSet operator()(std::monostate& pool)
        {
            assert(false&&"pool is std::monostate");
            return {};
        }
    };
    std::variant<std::monostate,vk::DynamicDescriptorPool> m_DescriptorPool;
};
}