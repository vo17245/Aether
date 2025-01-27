#pragma once
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <vector>
#include "Core/Math.h"
namespace Aether {
enum class BufferElementFormat : int32_t
{
    None = VK_FORMAT_UNDEFINED,
    Float32 = VK_FORMAT_R32_SFLOAT,
    Vec2f = VK_FORMAT_R32G32_SFLOAT,
    Vec3f = VK_FORMAT_R32G32B32_SFLOAT,
    Vec4f = VK_FORMAT_R32G32B32A32_SFLOAT,

};

inline uint32_t BufferElementFormatSize(BufferElementFormat format)
{
    switch (format)
    {
    case BufferElementFormat::Float32:
        return 4;
    case BufferElementFormat::Vec2f:
        return 8;
    case BufferElementFormat::Vec3f:
        return 12;
    case BufferElementFormat::Vec4f:
        return 16;
    default:
        assert(false&&"Unknown format");
        return 0;
    }
};
namespace ShaderType {
struct Float32
{};
struct Vec2f
{};
struct Vec3f
{};
struct Vec4f
{};
struct Mat2f
{};
struct Mat4f
{};
struct Mat3f
{};
} // namespace ShaderType
template <typename T>
uint32_t ShaderTypeSize()
{
    if constexpr (std::is_same_v<T, ShaderType::Float32>)
    {
        return 4;
    }
    else if constexpr (std::is_same_v<T, ShaderType::Vec2f>)
    {
        return 8;
    }
    else if constexpr (std::is_same_v<T, ShaderType::Vec3f>)
    {
        return 12;
    }
    else if constexpr (std::is_same_v<T, ShaderType::Vec4f>)
    {
        return 16;
    }
    else
    {
        assert(false&&"Unknown shader type");
        return 0;
    }
}
template <typename T>
BufferElementFormat ShaderTypeToFormat()
{
    if constexpr (std::is_same_v<T, ShaderType::Float32>)
    {
        return BufferElementFormat::Float32;
    }
    else if constexpr (std::is_same_v<T, ShaderType::Vec2f>)
    {
        return BufferElementFormat::Vec2f;
    }
    else if constexpr (std::is_same_v<T, ShaderType::Vec3f>)
    {
        return BufferElementFormat::Vec3f;
    }
    else if constexpr (std::is_same_v<T, ShaderType::Vec4f>)
    {
        return BufferElementFormat::Vec4f;
    }
    else
    {
        assert(false&&"Unknown shader type");
        return BufferElementFormat::Float32;
    }
}

template <typename T>
struct GetShaderType
{};
template <>
struct GetShaderType<float>
{
    using Type = ShaderType::Float32;
};
template <>
struct GetShaderType<Vec2f>
{
    using Type = ShaderType::Vec2f;
};
template <>
struct GetShaderType<Vec3f>
{
    using Type = ShaderType::Vec3f;
};
template <>
struct GetShaderType<Vec4f>
{
    using Type = ShaderType::Vec4f;
};
template <>
struct GetShaderType<Mat2f>
{
    using Type = ShaderType::Mat2f;
};
template <>
struct GetShaderType<Mat3f>
{
    using Type = ShaderType::Mat3f;
};
template <>
struct GetShaderType<Mat4f>
{
    using Type = ShaderType::Mat4f;
};

class VertexBufferLayout
{
public:
    struct Attribute
    {
        uint32_t location;
        uint32_t offset;// offset in element, *NOT* in vertex buffer
        BufferElementFormat format;
        bool operator==(const Attribute& other) const
        {
            return location == other.location && offset == other.offset && format == other.format;
        }
    };

public:
    /**
     * @note
     *    功能是vulkan的子集，attribute紧密排列
     *    如果需要更多的功能，可以创建一个空的VertexBufferLayout，然后调用PushAttribute和SetBinding
     */
    class Builder
    {
    public:
        Builder& Binding(uint32_t binding)
        {
            m_Binding = binding;
            return *this;
        }

        template <typename T>
        Builder& Push()
        {
            using ShaderType = typename GetShaderType<T>::Type;
            m_Attributes.emplace_back(Attribute{m_CurLocation, m_Offset, ShaderTypeToFormat<ShaderType>()});
            m_CurLocation++;
            m_Offset += ShaderTypeSize<ShaderType>();
            return *this;
        }

        VertexBufferLayout Build()
        {
            return VertexBufferLayout(m_Binding, std::move(m_Attributes));
        }

    private:
        std::vector<Attribute> m_Attributes;
        uint32_t m_Binding;
        uint32_t m_CurLocation = 0;
        uint32_t m_Offset = 0;
    };
    bool operator==(const VertexBufferLayout& other) const
    {
        // sort attribute by location
        auto sortedAttributes = m_Attributes;
        std::sort(sortedAttributes.begin(), sortedAttributes.end(), [](const Attribute& a, const Attribute& b) {
            return a.location < b.location;
        });
        auto otherSortedAttributes = other.m_Attributes;
        std::sort(otherSortedAttributes.begin(), otherSortedAttributes.end(), [](const Attribute& a, const Attribute& b) {
            return a.location < b.location;
        });
        return m_Binding == other.m_Binding && sortedAttributes == otherSortedAttributes;
    }
    bool operator!=(const VertexBufferLayout& other) const
    {
        return !(*this == other);
    }

public:
    VertexBufferLayout(uint32_t binding, std::vector<Attribute>&& attributes) :
        m_Binding(binding),
        m_Attributes(std::move(attributes))
    {
    }
    VertexBufferLayout() = default;

public:
    uint32_t CalculateStride() const
    {
        uint32_t stride = 0;
        for (const auto& attribute : m_Attributes)
        {
            stride += BufferElementFormatSize(attribute.format);
        }
        return stride;
    }
    uint32_t GetBinding() const
    {
        return m_Binding;
    }
    const std::vector<Attribute>& GetAttributes() const
    {
        return m_Attributes;
    }
    VkVertexInputBindingDescription CreateVulkanBindingDescription() const
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = m_Binding;
        bindingDescription.stride = CalculateStride();
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    std::vector<VkVertexInputAttributeDescription> CreateVulkanAttributeDescriptions() const
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        for (const auto& attribute : m_Attributes)
        {
            VkVertexInputAttributeDescription desc{};
            desc.binding = m_Binding;
            desc.location = attribute.location;
            desc.format = static_cast<VkFormat>(attribute.format);
            desc.offset = attribute.offset;
            attributeDescriptions.push_back(desc);
        }
        return attributeDescriptions;
    }
    void SetBinding(uint32_t binding)
    {
        m_Binding = binding;
    }
    void PushAttribute(const Attribute& attribute)
    {
        m_Attributes.push_back(attribute);
    }
    void ClearAttributes()
    {
        m_Attributes.clear();
    }

private:
    uint32_t m_Binding = 0;
    std::vector<Attribute> m_Attributes;
};
} // namespace Aether