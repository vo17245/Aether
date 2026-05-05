#pragma once
#include "Backend/Vulkan/DescriptorSet.h"
#include "Backend/Vulkan/DynamicDescriptorPool.h"
#include <variant>
namespace Aether::rhi
{
class DescriptorSet
{
public:
    DescriptorSet() = default;
    DescriptorSet(const DescriptorSet& other) = delete;
    DescriptorSet(DescriptorSet&& other) noexcept = default;
    DescriptorSet& operator=(const DescriptorSet& other) = delete;
    DescriptorSet& operator=(DescriptorSet&& other) noexcept = default;
    ~DescriptorSet() = default;
    DescriptorSet(vk::DynamicDescriptorPool::DescriptorResource&& descriptorResource) :
        m_DescriptorSet(std::move(descriptorResource))
    {
    }

public:
    static DescriptorSet Create(uint32_t samplerCount, uint32_t uboCount, uint32_t ssboCount);

private:
    std::variant<std::monostate, vk::DynamicDescriptorPool::DescriptorResource> m_DescriptorSet;
};
} // namespace Aether::rhi