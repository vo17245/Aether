#pragma once
#include <cstdint>
namespace Aether::TaskGraph
{
    enum class ResourceType : uint32_t
    {
        Unknown = 0,
        Buffer ,
        Texture ,
        FrameBuffer ,
    };
}