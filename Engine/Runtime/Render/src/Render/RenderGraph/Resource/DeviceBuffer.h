#pragma once
#include <Render/RenderApi.h>
#include "Resource.h"
#include <Core/Core.h>
namespace Aether::RenderGraph
{
enum class BufferType : uint32_t
{
    SSBO,
    UBO,
    VBO,
    Staging
};
struct BufferDesc
{
    BufferType type;
    size_t size; // in bytes
    bool operator==(const BufferDesc& other) const
    {
        return type == other.type && size == other.size;
    }
};
template <>
struct ResourceDescType<DeviceBuffer>
{
    using Type = BufferDesc;
};
template <>
struct Realize<DeviceBuffer>
{
    Scope<DeviceBuffer> operator()(const BufferDesc& desc)
    {
        switch (desc.type)
        {
        case BufferType::Staging: {
            auto deviceBuffer = DeviceBuffer::CreateForStaging(desc.size);
            if (!deviceBuffer)
            {
                return nullptr;
            }
            return CreateScope<DeviceBuffer>(std::move(deviceBuffer));
        }

        break;
        case BufferType::SSBO: {
            auto deviceBuffer = DeviceBuffer::CreateForSSBO(desc.size);
            if (!deviceBuffer)
            {
                return nullptr;
            }
            return CreateScope<DeviceBuffer>(std::move(deviceBuffer));
        }
        break;
        case BufferType::UBO: {
            auto deviceBuffer = DeviceBuffer::CreateForUniform(desc.size);

            if (!deviceBuffer)
            {
                return nullptr;
            }
            return CreateScope<DeviceBuffer>(std::move(deviceBuffer));
        }
        break;
        case BufferType::VBO: {
            auto deviceBuffer = DeviceBuffer::CreateForVBO(desc.size);
            if (!deviceBuffer)
            {
                return nullptr;
            }
            return CreateScope<DeviceBuffer>(std::move(deviceBuffer));
        }
        break;
        default:
            assert(false && "Unknown buffer type");
            return nullptr;
        }
    }
};
} // namespace Aether::RenderGraph

namespace Aether
{
template <>
struct Hash<RenderGraph::BufferDesc>
{
    std::size_t operator()(const RenderGraph::BufferDesc& value) const
    {
        return std::hash<uint32_t>()(static_cast<uint32_t>(value.type)) ^ std::hash<size_t>()(value.size);
    }
};
} // namespace Aether