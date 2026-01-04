#include "Render/RHI/Backend/Vulkan/DescriptorSet.h"
#include "Render/RHI/Backend/Vulkan/GlobalRenderContext.h"

namespace Aether {
namespace vk {

void DescriptorSetOperator::Apply()
{
    vkUpdateDescriptorSets(GRC::GetDevice(), m_Writes.size(), m_Writes.data(), 0, nullptr);
}

void DescriptorSetOperator::BindSSBO(uint32_t binding, const vk::Buffer& buffer)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_Set.GetHandle();
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    Scope<VkDescriptorBufferInfo> bufferInfoPtr = CreateScope<VkDescriptorBufferInfo>();
    auto& bufferInfo = *bufferInfoPtr;
    bufferInfo = {};
    bufferInfo.buffer = buffer.GetHandle();
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;
    write.pBufferInfo = &bufferInfo;
    m_Writes.push_back(write);
    m_BufferInfos.emplace_back(std::move(bufferInfoPtr));
}

void DescriptorSetOperator::BindSampler(uint32_t binding, const vk::Sampler& sampler, const vk::ImageView& imageView)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_Set.GetHandle();
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    Scope<VkDescriptorImageInfo> imageInfoPtr = CreateScope<VkDescriptorImageInfo>();
    auto& imageInfo = *imageInfoPtr;
    imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView.GetHandle();
    imageInfo.sampler = sampler.GetHandle();
    write.pImageInfo = imageInfoPtr.get();
    m_Writes.push_back(write);
    m_ImageInfos.emplace_back(std::move(imageInfoPtr));
}

void DescriptorSetOperator::BindUBO(uint32_t binding, const vk::Buffer& buffer)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_Set.GetHandle();
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    Scope<VkDescriptorBufferInfo> bufferInfoPtr = CreateScope<VkDescriptorBufferInfo>();
    auto& bufferInfo = *bufferInfoPtr;
    bufferInfo = {};
    bufferInfo.buffer = buffer.GetHandle();
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;
    write.pBufferInfo = bufferInfoPtr.get();
    m_Writes.push_back(write);
    m_BufferInfos.emplace_back(std::move(bufferInfoPtr));
}
std::optional<DescriptorSet> DescriptorSet::Create(const DescriptorSetLayout& layout, const DescriptorPool& pool)
{
    auto descriptorPool = pool.GetHandle();
    auto descriptorSetLayout = layout.GetHandle();
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    const VkDescriptorSetLayout layouts[] = {descriptorSetLayout};
    allocInfo.pSetLayouts = layouts;

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(GRC::GetDevice(), &allocInfo, &descriptorSet);
    if (result != VK_SUCCESS)
    {
        assert(false && "Failed to allocate descriptor set!");
        return std::nullopt;
    }
    return DescriptorSet(descriptorSet, descriptorPool);
}
Ref<DescriptorSet> DescriptorSet::CreateRef(const DescriptorSetLayout& layout, const DescriptorPool& pool)
{
    auto opt = Create(layout, pool);
    if (!opt.has_value())
    {
        return nullptr;
    }
    return Aether::CreateRef<DescriptorSet>(std::move(opt.value()));
}
Scope<DescriptorSet> DescriptorSet::CreateScope(const DescriptorSetLayout& layout, const DescriptorPool& pool)
{
    auto opt = Create(layout, pool);
    if (!opt.has_value())
    {
        return nullptr;
    }
    return Aether::CreateScope<DescriptorSet>(std::move(opt.value()));
}

VkDescriptorSet DescriptorSet::GetHandle() const
{
    return m_DescriptorSet;
}

DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
{
    m_DescriptorSet = other.m_DescriptorSet;
    m_DescriptorPool = other.m_DescriptorPool;
    other.m_DescriptorSet = VK_NULL_HANDLE;
    other.m_DescriptorPool = VK_NULL_HANDLE;
}
DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
{
    if (this != &other)
    {
        // if (m_DescriptorSet != VK_NULL_HANDLE)
        //     vkFreeDescriptorSets(GRC::GetDevice(), m_DescriptorPool, 1, &m_DescriptorSet);
        m_DescriptorSet = other.m_DescriptorSet;
        m_DescriptorPool = other.m_DescriptorPool;
        other.m_DescriptorSet = VK_NULL_HANDLE;
        other.m_DescriptorPool = VK_NULL_HANDLE;
    }
    return *this;
}
}
} // namespace Aether::vk