#pragma once
#include <cassert>
#include <expected>
#include <variant>
#include "../Vulkan/DynamicDescriptorPool.h"
#include "Render/Config.h"
namespace Aether
{
class DeviceDescriptorSet
{
public:
    DeviceDescriptorSet(vk::DynamicDescriptorPool::DescriptorResource&& resource) :
        m_DescriptorSet(std::move(resource))
    {
    }
    operator bool() const
    {
        return m_DescriptorSet.index() != 0;
    }
    DeviceDescriptorSet()=default;
    vk::DynamicDescriptorPool::DescriptorResource& GetVk()
    {
        assert(std::holds_alternative<vk::DynamicDescriptorPool::DescriptorResource>(m_DescriptorSet) && "not a vk descriptor set");
        return std::get<vk::DynamicDescriptorPool::DescriptorResource>(m_DescriptorSet);
    }
    const vk::DynamicDescriptorPool::DescriptorResource& GetVk() const
    {
        assert(std::holds_alternative<vk::DynamicDescriptorPool::DescriptorResource>(m_DescriptorSet) && "not a vk descriptor set");
        return std::get<vk::DynamicDescriptorPool::DescriptorResource>(m_DescriptorSet);
    }
    bool Empty()
    {
        return m_DescriptorSet.index() == 0;
    }
private:
    std::variant<std::monostate, vk::DynamicDescriptorPool::DescriptorResource> m_DescriptorSet;
};

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
    void Clear()
    {
        std::visit(ClearImpl{}, m_DescriptorPool);
    }
    DeviceDescriptorPool(DeviceDescriptorPool&& other)
    {
        m_DescriptorPool = std::move(other.m_DescriptorPool);
        other.m_DescriptorPool = std::monostate();
    }
    DeviceDescriptorPool& operator=(DeviceDescriptorPool&& other)
    {
        if (this != &other)
        {
            m_DescriptorPool = std::move(other.m_DescriptorPool);
            other.m_DescriptorPool = std::monostate();
        }
        return *this;
    }
    DeviceDescriptorPool(const DeviceDescriptorPool&) = delete;
    DeviceDescriptorPool& operator=(const DeviceDescriptorPool&) = delete;
    DeviceDescriptorPool() = default;
private:
    struct ClearImpl
    {
        void operator()(std::monostate& pool)
        {
            assert(false && "not implemented");
        }
        void operator()(vk::DynamicDescriptorPool& pool)
        {
            pool.Clear();
        }
    };
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
    assert(false && "not implemented");
    return {};
}
} // namespace Aether