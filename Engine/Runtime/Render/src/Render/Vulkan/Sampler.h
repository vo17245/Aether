#pragma once
#include "vulkan/vulkan_core.h"
#include <optional>
#include <vulkan/vulkan.h>
#include "ImageView.h"
namespace Aether {
namespace vk {

class Sampler
{
public:
    enum class Filter : int32_t
    {
        Nearest = VK_FILTER_NEAREST,
        Linear = VK_FILTER_LINEAR,
    };

public:
    class Builder
    {
    public:
        Builder();
        Builder& SetMagFilter(Filter filter);
        Builder& SetMinFilter(Filter filter);
        Builder& SetMipmapMode(VkSamplerMipmapMode mode);
        Builder& SetAddressMode(VkSamplerAddressMode mode);
        Builder& SetAnisotropyEnable(bool enable);
        Builder& SetMaxAnisotropy(float maxAnisotropy);
        Builder& SetBorderColor(VkBorderColor color);
        Builder& SetUnnormalizedCoordinates(bool enable);
        Builder& SetCompareEnable(bool enable);
        Builder& SetCompareOp(VkCompareOp op);
        Builder& SetMinLod(float minLod);
        Builder& SetMaxLod(float maxLod);
        Builder& SetMipLodBias(float mipLodBias);
        Builder& SetDefaultCreateInfo();
        std::optional<Sampler> Build();

    private:
        void InitCreateInfo();
        VkSamplerCreateInfo m_CreateInfo{};
    };
    VkSampler GetHandle() const;
    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;
    Sampler(Sampler&& other) noexcept;
    Sampler& operator=(Sampler&& other) noexcept;
    ~Sampler();
    /**
     *@param binding - Sampler binding in descriptor set
     */

private:
    Sampler(VkSampler handle);

    VkSampler m_Handle;
};
}
} // namespace Aether::vk