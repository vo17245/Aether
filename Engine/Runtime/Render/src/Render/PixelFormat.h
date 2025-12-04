#pragma once
#include <cstdint>
#include <format>
#include "Core/Base.h"
#include "Render/PixelFormat.h"
#include "vulkan/vulkan_core.h"
#include <cassert>

namespace Aether
{
// clang-format off
enum class PixelFormat : int32_t
{
    // 线性颜色空间
    RGBA8888 = Bit(0),
    RGB888 = Bit(1),
    RG88 = Bit(2),
    R8 = Bit(3),
    RGBA_FLOAT32 = Bit(4),
    RGB_FLOAT32 = Bit(5),
    RG_FLOAT32 = Bit(6),
    R_FLOAT32 = Bit(7),
    RGBA_FLOAT16 = Bit(8),
    RGB_FLOAT16 = Bit(9),
    RG_FLOAT16 = Bit(10),
    R_FLOAT16 = Bit(11),
    
    // 过了一遍gamma校正的颜色空间
    RGBA8888_SRGB = Bit(12),
    RGB888_SRGB = Bit(13),
    RGBA_FLOAT32_SRGB = Bit(14),
    RGB_FLOAT32_SRGB = Bit(15),
    // 不知道有没有gamma校准的颜色空间，为上面两个中的一种
    RGBA8888_UNKNOWN = Bit(16),
    RGB888_UNKNOWN = Bit(17),
    RGBA_FLOAT32_UNKNOWN = Bit(18),
    RGB_FLOAT32_UNKNOWN = Bit(19),
    // 给深度缓冲使用
    R_FLOAT32_DEPTH = Bit(20),
    // shader采样时，获取的类型是uint(一般是32位)
    RGBA8888_UInt = Bit(21),
};
inline constexpr const bool PixelFormatIsFloat(PixelFormat format)
{
    constexpr const int32_t mask = (int32_t)PixelFormat::RGBA_FLOAT32
                                   | (int32_t)PixelFormat::RGB_FLOAT32
                                   | (int32_t)PixelFormat::RG_FLOAT32
                                   | (int32_t)PixelFormat::R_FLOAT32
                                   | (int32_t)PixelFormat::RGBA_FLOAT32_SRGB
                                   | (int32_t)PixelFormat::RGB_FLOAT32_SRGB
                                   | (int32_t)PixelFormat::RGBA_FLOAT32_UNKNOWN
                                   | (int32_t)PixelFormat::RGB_FLOAT32_UNKNOWN
                                   | (int32_t)PixelFormat::R_FLOAT32_DEPTH;
    return (int32_t)format & mask;
}

inline constexpr const uint32_t PixelFormatSize(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::RGBA8888: return 4;
    case PixelFormat::RGB888: return 3;
    case PixelFormat::RG88: return 2;
    case PixelFormat::R8: return 1;
    case PixelFormat::RGBA_FLOAT32: return 16;
    case PixelFormat::RGB_FLOAT32: return 12;
    case PixelFormat::RG_FLOAT32: return 8;
    case PixelFormat::R_FLOAT32: return 4;
    case PixelFormat::RGBA8888_SRGB: return 4;
    case PixelFormat::RGB888_SRGB: return 3;
    case PixelFormat::RGBA_FLOAT32_SRGB: return 16;
    case PixelFormat::RGB_FLOAT32_SRGB: return 12;
    case PixelFormat::RGBA8888_UNKNOWN: return 4;
    case PixelFormat::RGB888_UNKNOWN: return 3;
    case PixelFormat::RGBA_FLOAT32_UNKNOWN: return 16;
    case PixelFormat::R_FLOAT32_DEPTH: return 4;
    case PixelFormat::RGBA8888_UInt: return 4;
    case PixelFormat::RGBA_FLOAT16: return 8;
    case PixelFormat::RGB_FLOAT16: return 6;
    case PixelFormat::RG_FLOAT16: return 4;
    case PixelFormat::R_FLOAT16: return 2;

    default:
        assert(false&&"Unknown PixelFormat");
        return 0;
    }
}

inline constexpr const char* PixelFormatToString(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::RGBA8888: return "RGBA8888";
    case PixelFormat::RGB888: return "RGB888";
    case PixelFormat::RG88: return "RG88";
    case PixelFormat::R8: return "R8";
    case PixelFormat::RGBA_FLOAT32: return "RGBA_FLOAT32";
    case PixelFormat::RGB_FLOAT32: return "RGB_FLOAT32";
    case PixelFormat::RG_FLOAT32: return "RG_FLOAT32";
    case PixelFormat::R_FLOAT32: return "R_FLOAT32";
    case PixelFormat::RGBA8888_SRGB: return "RGBA8888_SRGB";
    case PixelFormat::RGB888_SRGB: return "RGB888_SRGB";
    case PixelFormat::RGBA_FLOAT32_SRGB: return "RGBA_FLOAT32_SRGB";
    case PixelFormat::RGB_FLOAT32_SRGB: return "RGB_FLOAT32_SRGB";
    case PixelFormat::RGBA8888_UNKNOWN: return "RGBA8888_UNKNOWN";
    case PixelFormat::RGB888_UNKNOWN: return "RGB888_UNKNOWN";
    case PixelFormat::RGBA_FLOAT32_UNKNOWN: return "RGBA_FLOAT32_UNKNOWN";
    case PixelFormat::R_FLOAT32_DEPTH: return "R_FLOAT32_DEPTH";
    case PixelFormat::RGBA8888_UInt: return "RGBA8888_UInt";
    case PixelFormat::RGBA_FLOAT16: return "RGBA_FLOAT16";
    case PixelFormat::RGB_FLOAT16: return "RGB_FLOAT16";
    case PixelFormat::RG_FLOAT16: return "RG_FLOAT16";
    case PixelFormat::R_FLOAT16: return "R_FLOAT16";
    default:
        assert(false&&"Unknown PixelFormat");
        return "Unknown PixelFormat";
    }
}

inline constexpr VkFormat PixelFormatToVkFormat(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::RGBA8888: return VK_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::RGB888: return VK_FORMAT_R8G8B8_UNORM;
    case PixelFormat::RG88: return VK_FORMAT_R8G8_UNORM;
    case PixelFormat::R8: return VK_FORMAT_R8_UNORM;
    case PixelFormat::RGBA_FLOAT32: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case PixelFormat::RGB_FLOAT32: return VK_FORMAT_R32G32B32_SFLOAT;
    case PixelFormat::RG_FLOAT32: return VK_FORMAT_R32G32_SFLOAT;
    case PixelFormat::R_FLOAT32: return VK_FORMAT_R32_SFLOAT;
    case PixelFormat::RGBA8888_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
    case PixelFormat::RGB888_SRGB: return VK_FORMAT_R8G8B8_SRGB;
    case PixelFormat::RGBA_FLOAT32_SRGB: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case PixelFormat::RGB_FLOAT32_SRGB: return VK_FORMAT_R32G32B32_SFLOAT;
    case PixelFormat::RGBA8888_UNKNOWN: return VK_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::RGB888_UNKNOWN: return VK_FORMAT_R8G8B8_UNORM;
    case PixelFormat::RGBA_FLOAT32_UNKNOWN: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case PixelFormat::R_FLOAT32_DEPTH: return VK_FORMAT_D32_SFLOAT;
    case PixelFormat::RGBA8888_UInt: return VK_FORMAT_R8G8B8A8_UINT;
    case PixelFormat::RGBA_FLOAT16: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case PixelFormat::RGB_FLOAT16: return VK_FORMAT_R16G16B16_SFLOAT;
    case PixelFormat::RG_FLOAT16: return VK_FORMAT_R16G16_SFLOAT;
    case PixelFormat::R_FLOAT16: return VK_FORMAT_R16_SFLOAT;
    
    default:
        assert(false&&"Unknown PixelFormat: {}");
        return VK_FORMAT_UNDEFINED;
    }
}
// clang-format on
} // namespace Aether
