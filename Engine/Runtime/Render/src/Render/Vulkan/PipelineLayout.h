#pragma once

#include "Core/Base.h"
#include "vulkan/vulkan_core.h"
#include <optional>
#include "DescriptorSetLayout.h"
#include "Def.h"
namespace Aether {
namespace vk {

class PipelineLayout
{
public:
    class Builder
    {
    public:
        Builder& AddDescriptorSetLayouts(std::span<DescriptorSetLayout> layouts);
        Builder& AddDescriptorSetLayout(const DescriptorSetLayout& layout);
        Builder& AddPushConstantRange(uint32_t size, vk::ShaderStageFlags stages, uint32_t offset = 0);
        std::optional<PipelineLayout> Build() const;
        Scope<PipelineLayout> BuildScope() const;
        Ref<PipelineLayout> BuildRef() const;

    private:
        std::vector<VkDescriptorSetLayout> m_Layouts;
        std::vector<VkPushConstantRange> m_Ranges;
    };
    static std::optional<PipelineLayout> Create();

    ~PipelineLayout();
    VkPipelineLayout GetHandle() const;
    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout& operator=(const PipelineLayout&) = delete;
    PipelineLayout(PipelineLayout&& other) noexcept;
    PipelineLayout& operator=(PipelineLayout&& other) noexcept;

private:
    PipelineLayout(VkPipelineLayout pipelineLayout) :
        m_PipelineLayout(pipelineLayout)
    {
    }
    VkPipelineLayout m_PipelineLayout;
};
}
} // namespace Aether::vk