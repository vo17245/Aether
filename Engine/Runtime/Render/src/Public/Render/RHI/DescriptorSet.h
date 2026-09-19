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
    static DescriptorSet CreateForFrame(uint32_t frameIndex,
                                        uint32_t samplerCount,
                                        uint32_t uboCount,
                                        uint32_t ssboCount);
    bool Empty() const
    {
        return std::holds_alternative<std::monostate>(m_DescriptorSet);
    }
    explicit operator bool() const { return !Empty(); }
    vk::DynamicDescriptorPool::DescriptorResource& GetVk()
    {
        return std::get<vk::DynamicDescriptorPool::DescriptorResource>(m_DescriptorSet);
    }
private:
    std::variant<std::monostate, vk::DynamicDescriptorPool::DescriptorResource> m_DescriptorSet;
};
} // namespace Aether::rhi
