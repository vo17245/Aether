#pragma once
#include <vulkan/vulkan.h>
#include <type_traits>
namespace Aether {
namespace vk {
enum class ShaderStage:uint32_t
{
    None = 0,
    Vertex = VK_SHADER_STAGE_VERTEX_BIT,
    TessellationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
    Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
    Compute = VK_SHADER_STAGE_COMPUTE_BIT,
    AllGraphics = VK_SHADER_STAGE_ALL_GRAPHICS,
    All = VK_SHADER_STAGE_ALL
};
using ShaderStageFlags = uint32_t;

// Enable bit operations for ShaderStage
inline ShaderStage operator|(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(static_cast<std::underlying_type<ShaderStage>::type>(a) | static_cast<std::underlying_type<ShaderStage>::type>(b));
}

inline ShaderStage operator&(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(static_cast<std::underlying_type<ShaderStage>::type>(a) & static_cast<std::underlying_type<ShaderStage>::type>(b));
}

inline ShaderStage operator~(ShaderStage a)
{
    return static_cast<ShaderStage>(~static_cast<std::underlying_type<ShaderStage>::type>(a));
}

inline ShaderStage& operator|=(ShaderStage& a, ShaderStage b)
{
    return a = a | b;
}

inline ShaderStage& operator&=(ShaderStage& a, ShaderStage b)
{
    return a = a & b;
}
}
} // namespace Aether::vk