#include "Render/RHI/Backend/Vulkan/DescriptorSetLayout.h"
#include "Render/RHI/Backend/Vulkan/GlobalRenderContext.h"

namespace Aether {
namespace vk {
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::BeginUniformBinding()
{
    memset(&m_Binding, 0, sizeof(VkDescriptorSetLayoutBinding));
    m_Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_Binding.binding = m_BindingIndex++;
    m_Binding.descriptorCount = 1;
    m_Binding.stageFlags = 0;
    m_Binding.pImmutableSamplers = nullptr;
    return *this;
}
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::UseVertexStage()
{
    m_Binding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    return *this;
}
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::UseFragmentStage()
{
    m_Binding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    return *this;
}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::EndUniformBinding()
{
    m_Bindings.push_back(m_Binding);
    return *this;
}
std::optional<DescriptorSetLayout> DescriptorSetLayout::Builder::Build()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
    layoutInfo.pBindings = m_Bindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(GRC::GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return DescriptorSetLayout(descriptorSetLayout);
}
Scope<DescriptorSetLayout> DescriptorSetLayout::Builder::BuildScope()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
    layoutInfo.pBindings = m_Bindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(GRC::GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        return nullptr;
    }
    auto* p = new DescriptorSetLayout(descriptorSetLayout);
    return Scope<DescriptorSetLayout>(p);
}
Ref<DescriptorSetLayout> DescriptorSetLayout::Builder::BuildRef()
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
    layoutInfo.pBindings = m_Bindings.data();

    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(GRC::GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        return nullptr;
    }
    auto* p = new DescriptorSetLayout(descriptorSetLayout);
    return Ref<DescriptorSetLayout>(p);
}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::BeginSamplerBinding()
{
    memset(&m_Binding, 0, sizeof(VkDescriptorSetLayoutBinding));
    m_Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    m_Binding.binding = m_BindingIndex++;
    m_Binding.descriptorCount = 1;
    m_Binding.stageFlags = 0;
    m_Binding.pImmutableSamplers = nullptr;
    return *this;
}
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::EndSamplerBinding()
{
    m_Bindings.push_back(m_Binding);
    return *this;
}
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::BeginSSBO()
{
    memset(&m_Binding, 0, sizeof(VkDescriptorSetLayoutBinding));
    m_Binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    m_Binding.binding = m_BindingIndex++;
    m_Binding.descriptorCount = 1;
    m_Binding.stageFlags = 0;
    m_Binding.pImmutableSamplers = nullptr;
    return *this;
}
DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::EndSSBO()
{
    m_Bindings.push_back(m_Binding);
    return *this;
}
DescriptorSetLayout::~DescriptorSetLayout()
{
    if (m_Layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(GRC::GetDevice(), m_Layout, nullptr);
}
VkDescriptorSetLayout DescriptorSetLayout::GetHandle() const
{
    return m_Layout;
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
{
    m_Layout = other.m_Layout;
    other.m_Layout = VK_NULL_HANDLE;
}
DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept
{
    if (this != &other)
    {
        if (m_Layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(GRC::GetDevice(), m_Layout, nullptr);
        m_Layout = other.m_Layout;
        other.m_Layout = VK_NULL_HANDLE;
    }
    return *this;
}
}
} // namespace Aether::vk