#include "DescriptorPool.h"
#include "GlobalRenderContext.h"

namespace Aether {
namespace vk {
/**
    @brief
        一个pool中可以包含多个UnifomBuffer，多个Sampler
        Push 函数指定有多少个UnifomBuffer，多少个Sampler
    */
DescriptorPool::Builder& DescriptorPool::Builder::Push(size_t size, Type type)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = static_cast<VkDescriptorType>(type);
    poolSize.descriptorCount = size;
    m_PoolSizes.push_back(poolSize);
    return *this;
}
DescriptorPool::Builder& DescriptorPool::Builder::PushUBO(size_t count)
{
    return Push(count, Type::UniformBuffer);
}
DescriptorPool::Builder& DescriptorPool::Builder::PushSampler(size_t count)
{
    return Push(count, Type::Sampler);
}
DescriptorPool::Builder& DescriptorPool::Builder::PushSSBO(size_t count)
{
    return Push(count, Type::SSBO);
}
DescriptorPool::Builder& DescriptorPool::Builder::MaxSets(size_t count)
{
    m_MaxSets = count;
    return *this;
}
std::optional<DescriptorPool> DescriptorPool::Builder::Build()
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
    poolInfo.pPoolSizes = m_PoolSizes.data();
    poolInfo.maxSets = m_MaxSets;

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(GRC::GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return DescriptorPool(descriptorPool);
}
Scope<DescriptorPool> DescriptorPool::Builder::BuildScope()
{
    auto opt = Build();
    if (!opt.has_value())
    {
        return nullptr;
    }
    return CreateScope<DescriptorPool>(std::move(opt.value()));
}
Ref<DescriptorPool> DescriptorPool::Builder::BuildRef()
{
    auto opt = Build();
    if (!opt.has_value())
    {
        return nullptr;
    }
    return CreateRef<DescriptorPool>(std::move(opt.value()));
}

DescriptorPool::~DescriptorPool()
{
    if (m_DescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(GRC::GetDevice(), m_DescriptorPool, nullptr);
}
VkDescriptorPool DescriptorPool::GetHandle() const
{
    return m_DescriptorPool;
}

DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
{
    m_DescriptorPool = other.m_DescriptorPool;
    other.m_DescriptorPool = VK_NULL_HANDLE;
}
DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
{
    if (this != &other)
    {
        if (m_DescriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(GRC::GetDevice(), m_DescriptorPool, nullptr);
        m_DescriptorPool = other.m_DescriptorPool;
        other.m_DescriptorPool = VK_NULL_HANDLE;
    }
    return *this;
}
void DescriptorPool::ClearDescriptorSet()
{
    vkResetDescriptorPool(GRC::GetDevice(), m_DescriptorPool, 0);
}
}} // namespace Aether::vk