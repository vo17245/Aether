#include "PipelineLayout.h"
#include "Def.h"
#include "DescriptorSetLayout.h"
#include "GlobalRenderContext.h"
#include <span>
namespace Aether {
namespace vk {
PipelineLayout::Builder& PipelineLayout::Builder::AddDescriptorSetLayout(const DescriptorSetLayout& layout)
{
    m_Layouts.emplace_back(layout.GetHandle());
    return *this;
}
PipelineLayout::Builder& PipelineLayout::Builder::AddDescriptorSetLayouts(std::span<DescriptorSetLayout> layouts)
{
    for (const auto& layout : layouts)
    {
        m_Layouts.emplace_back(layout.GetHandle());
    }
    return *this;
}
PipelineLayout::Builder& PipelineLayout::Builder::AddPushConstantRange(uint32_t size, vk::ShaderStageFlags stages, uint32_t offset)
{
#if AETHER_RUNTIME_CHECK
    if (size > 128)
    {
        assert(false && "PushConstantRange size should be less than 128 bytes");
    }
#endif
    VkPushConstantRange range = {};
    range.stageFlags = static_cast<VkShaderStageFlags>(stages);
    range.offset = offset;
    range.size = size;
    m_Ranges.emplace_back(range);
    return *this;
}
std::optional<PipelineLayout> PipelineLayout::Builder::Build() const
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_Layouts.size());
    pipelineLayoutInfo.pSetLayouts = m_Layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_Ranges.size());
    pipelineLayoutInfo.pPushConstantRanges = m_Ranges.data();

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(GlobalRenderContext::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return PipelineLayout(pipelineLayout);
}
Scope<PipelineLayout> PipelineLayout::Builder::BuildScope() const
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_Layouts.size());
    pipelineLayoutInfo.pSetLayouts = m_Layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_Ranges.size());
    pipelineLayoutInfo.pPushConstantRanges = m_Ranges.data();

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(GlobalRenderContext::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        return nullptr;
    }
    auto* p = new PipelineLayout(pipelineLayout);
    return Scope<PipelineLayout>(p);
}
Ref<PipelineLayout> PipelineLayout::Builder::BuildRef() const
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_Layouts.size());
    pipelineLayoutInfo.pSetLayouts = m_Layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_Ranges.size());
    pipelineLayoutInfo.pPushConstantRanges = m_Ranges.data();

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(GlobalRenderContext::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        return nullptr;
    }
    auto* p = new PipelineLayout(pipelineLayout);
    return Ref<PipelineLayout>(p);
}
std::optional<PipelineLayout> PipelineLayout::Create()
{
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(GlobalRenderContext::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        return std::nullopt;
    }
    return PipelineLayout(pipelineLayout);
}
PipelineLayout::~PipelineLayout()
{
    if (m_PipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(GlobalRenderContext::GetDevice(), m_PipelineLayout, nullptr);
}
VkPipelineLayout PipelineLayout::GetHandle() const
{
    return m_PipelineLayout;
}

PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
{
    m_PipelineLayout = other.m_PipelineLayout;
    other.m_PipelineLayout = VK_NULL_HANDLE;
}
PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept
{
    if (this != &other)
    {
        if (m_PipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(GlobalRenderContext::GetDevice(), m_PipelineLayout, nullptr);
        m_PipelineLayout = other.m_PipelineLayout;
        other.m_PipelineLayout = VK_NULL_HANDLE;
    }
    return *this;
}
}
} // namespace Aether::vk