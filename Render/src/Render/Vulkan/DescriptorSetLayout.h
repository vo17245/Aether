#pragma once
#include <optional>
#include <stdint.h>
#include "Core/Base.h"
#include "vulkan/vulkan_core.h"
#include <vector>
namespace Aether {
namespace vk {

class DescriptorSetLayout
{
public:
    class Builder
    {
    private:
        enum class BindingType
        {
            UniformBuffer,
            CombinedImageSampler,
            StorageBuffer,
            StorageImage,
            InputAttachment
        };

    public:
        Builder& BeginUniformBinding();
        Builder& UseVertexStage();
        Builder& UseFragmentStage();

        Builder& EndUniformBinding();
        std::optional<DescriptorSetLayout> Build();
        Scope<DescriptorSetLayout> BuildScope();
        Ref<DescriptorSetLayout> BuildRef();

        Builder& BeginSamplerBinding();
        Builder& EndSamplerBinding();
        Builder& BeginSSBO();
        Builder& EndSSBO();
        void SetBindingIndex(uint32_t index)
        {
            m_BindingIndex = index;
        }
    private:
        VkDescriptorSetLayoutBinding m_Binding;
        uint32_t m_BindingIndex = 0;
        std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
    };

public:
    ~DescriptorSetLayout();
    VkDescriptorSetLayout GetHandle() const;
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

private:
    DescriptorSetLayout(VkDescriptorSetLayout layout) :
        m_Layout(layout)
    {
    }
    VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
};
}
} // namespace Aether::vk
