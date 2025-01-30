#pragma once
#include <cassert>
#include <expected>
#include <variant>
#include "../Vulkan/DynamicDescriptorPool.h"
#include "Render/Config.h"
namespace Aether
{
using DeviceDescriptorSet = std::variant<std::monostate, vk::DynamicDescriptorPool::DescriptorResource>;

class DeviceDescriptorPool
{
public:
    DeviceDescriptorSet CreateSet(size_t ubo, size_t ssbo, size_t sampler)
    {
        return std::visit(CreateSetImpl{ubo, ssbo, sampler}, m_DescriptorPool);
    }
    static std::expected<DeviceDescriptorPool, std::string> Create()
    {
        assert(Render::Config::RenderApi == Render::Api::Vulkan && "only support vulkan now");
        DeviceDescriptorPool pool;
        pool.m_DescriptorPool = vk::DynamicDescriptorPool();
        return pool;
    }
    operator bool() const
    {
        return m_DescriptorPool.index() != 0;
    }

private:
    struct CreateSetImpl
    {
        size_t ubo;
        size_t ssbo;
        size_t sampler;
        template <typename T>
        DeviceDescriptorSet Create(T& pool)
        {
            static_assert(sizeof(T) == 0, "not support");
            return {};
        }

        template <typename T>
        DeviceDescriptorSet operator()(T& pool)
        {
            return Create(pool);
        }
    };

private:
    std::variant<std::monostate, vk::DynamicDescriptorPool> m_DescriptorPool;
};
template <>
inline DeviceDescriptorSet DeviceDescriptorPool::CreateSetImpl::Create(vk::DynamicDescriptorPool& pool)
{
    auto set = pool.CreateSet(ubo, sampler, ssbo);
    if (!set)
    {
        return {};
    }
    return std::move(set.value());
}
template <>
inline DeviceDescriptorSet DeviceDescriptorPool::CreateSetImpl::Create(std::monostate& pool)
{
    assert(false&&"not implemented");
    return {};
}
} // namespace Aether