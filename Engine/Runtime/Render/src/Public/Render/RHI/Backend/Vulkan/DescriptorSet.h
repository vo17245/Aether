#pragma once
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"
#include "ImageView.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <optional>
#include <vector>
#include "Sampler.h"
#include "Buffer.h"
#include "Core/Base.h"
namespace Aether {
namespace vk {

/**
 * @brief
 * DescriptorSet 不会自动释放，通过DescriptorPool一次释放所有通过它创建的DescriptorSet
 */
class DescriptorSet
{
public:
    static std::optional<DescriptorSet> Create(const DescriptorSetLayout& layout, const DescriptorPool& pool);
    static Ref<DescriptorSet> CreateRef(const DescriptorSetLayout& layout, const DescriptorPool& pool);
    static Scope<DescriptorSet> CreateScope(const DescriptorSetLayout& layout, const DescriptorPool& pool);
    ~DescriptorSet()
    {
    }
    VkDescriptorSet GetHandle() const;
    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&& other) noexcept;
    DescriptorSet& operator=(DescriptorSet&& other) noexcept;

private:
    DescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorPool descriptorPool) :
        m_DescriptorSet(descriptorSet), m_DescriptorPool(descriptorPool)
    {
    }
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
};

class DescriptorSetOperator
{
public:
    DescriptorSetOperator(DescriptorSet& set) :
        m_Set(set)
    {
    }
    void Apply();
    void BindSSBO(uint32_t binding, const vk::Buffer& buffer);
    void BindSampler(uint32_t binding, const vk::Sampler& sampler, const vk::ImageView& imageView);
    void BindUBO(uint32_t binding, const vk::Buffer& buffer);

private:
    std::vector<VkWriteDescriptorSet> m_Writes;
    std::vector<Scope<VkDescriptorBufferInfo>> m_BufferInfos;
    std::vector<Scope<VkDescriptorImageInfo>> m_ImageInfos;
    DescriptorSet& m_Set;
};

}
} // namespace Aether::vk