#include "Sampler.h"
#include "GlobalRenderContext.h"
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {

Sampler::Builder::Builder()
{
    InitCreateInfo();
}
Sampler::Builder& Sampler::Builder::SetMagFilter(Filter filter)
{
    m_CreateInfo.magFilter = static_cast<VkFilter>(filter);
    return *this;
}
Sampler::Builder& Sampler::Builder::SetMinFilter(Filter filter)
{
    m_CreateInfo.minFilter = static_cast<VkFilter>(filter);
    return *this;
}
Sampler::Builder& Sampler::Builder::SetMipmapMode(VkSamplerMipmapMode mode)
{
    m_CreateInfo.mipmapMode = mode;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetAddressMode(VkSamplerAddressMode mode)
{
    m_CreateInfo.addressModeU = mode;
    m_CreateInfo.addressModeV = mode;
    m_CreateInfo.addressModeW = mode;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetAnisotropyEnable(bool enable)
{
    m_CreateInfo.anisotropyEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetMaxAnisotropy(float maxAnisotropy)
{
    m_CreateInfo.maxAnisotropy = maxAnisotropy;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetBorderColor(VkBorderColor color)
{
    m_CreateInfo.borderColor = color;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetUnnormalizedCoordinates(bool enable)
{
    m_CreateInfo.unnormalizedCoordinates = enable ? VK_TRUE : VK_FALSE;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetCompareEnable(bool enable)
{
    m_CreateInfo.compareEnable = enable ? VK_TRUE : VK_FALSE;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetCompareOp(VkCompareOp op)
{
    m_CreateInfo.compareOp = op;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetMinLod(float minLod)
{
    m_CreateInfo.minLod = minLod;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetMaxLod(float maxLod)
{
    m_CreateInfo.maxLod = maxLod;
    return *this;
}
Sampler::Builder& Sampler::Builder::SetMipLodBias(float mipLodBias)
{
    m_CreateInfo.mipLodBias = mipLodBias;
    return *this;
}
std::optional<Sampler> Sampler::Builder::Build()
{
    VkSampler textureSampler;
    if (vkCreateSampler(GRC::GetDevice(), &m_CreateInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        assert(false && "Failed to create texture sampler");
        return std::nullopt;
    }
    return Sampler(textureSampler);
}

void Sampler::Builder::InitCreateInfo()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GRC::GetPhysicalDevice(), &properties);

    m_CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    m_CreateInfo.magFilter = VK_FILTER_LINEAR;
    m_CreateInfo.minFilter = VK_FILTER_LINEAR;
    m_CreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    m_CreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    m_CreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    m_CreateInfo.anisotropyEnable = VK_TRUE;
    m_CreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    m_CreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    m_CreateInfo.unnormalizedCoordinates = VK_FALSE;
    m_CreateInfo.compareEnable = VK_FALSE;
    m_CreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    m_CreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    m_CreateInfo.mipLodBias = 0.0f;
    m_CreateInfo.minLod = 0.0f;
    m_CreateInfo.maxLod = 0.0f;
}

VkSampler Sampler::GetHandle() const
{
    return m_Handle;
}

Sampler::Sampler(VkSampler handle)
{
    m_Handle = handle;
}

Sampler::Builder& Sampler::Builder::SetDefaultCreateInfo()
{
    InitCreateInfo();
    return *this;
}

Sampler::Sampler(Sampler&& other) noexcept
{
    m_Handle = other.m_Handle;
    other.m_Handle = VK_NULL_HANDLE;
}
Sampler& Sampler::operator=(Sampler&& other) noexcept
{
    if (this != &other)
    {
        if (m_Handle != VK_NULL_HANDLE)
        {
            vkDestroySampler(GRC::GetDevice(), m_Handle, nullptr);
        }
        m_Handle = other.m_Handle;
        other.m_Handle = VK_NULL_HANDLE;
    }
    return *this;
}
Sampler::~Sampler()
{
    if (m_Handle != VK_NULL_HANDLE)
    {
        vkDestroySampler(GRC::GetDevice(), m_Handle, nullptr);
    }
}

}
} // namespace Aether::vk