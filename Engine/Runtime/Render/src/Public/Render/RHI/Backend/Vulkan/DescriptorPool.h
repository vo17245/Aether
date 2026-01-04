#pragma once
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <optional>
#include <type_traits>
#include "Core/Base.h"
#include <vector>
namespace Aether {
namespace vk {

class DescriptorPool
{
public:
    enum class Type : int32_t
    {
        UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        Sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        SSBO = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    };
    class Builder
    {
    public:
        /**
        @brief
            一个pool中可以包含多个UnifomBuffer，多个Sampler
            Push 函数指定有多少个UnifomBuffer，多少个Sampler
        */
        Builder& Push(size_t size, Type type);
        Builder& PushUBO(size_t count);
        Builder& PushSampler(size_t count);
        Builder& PushSSBO(size_t count);
        Builder& MaxSets(size_t count);
        std::optional<DescriptorPool> Build();
        Scope<DescriptorPool> BuildScope();
        Ref<DescriptorPool> BuildRef();

    private:
        std::vector<VkDescriptorPoolSize> m_PoolSizes;
        uint32_t m_MaxSets = 1;
    };

    ~DescriptorPool();
    VkDescriptorPool GetHandle() const;
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;
    DescriptorPool(DescriptorPool&& other) noexcept;
    DescriptorPool& operator=(DescriptorPool&& other) noexcept;
    void ClearDescriptorSet();

private:
    VkDescriptorPool m_DescriptorPool;
    DescriptorPool(VkDescriptorPool descriptorPool) :
        m_DescriptorPool(descriptorPool)
    {
    }
};
}
} // namespace Aether::vk