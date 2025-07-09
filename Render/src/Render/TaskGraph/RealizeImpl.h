#pragma once
#include <Render/RenderApi.h>
#include "Detail/Realize.h"
#include "ResourceImpl.h"
namespace Aether::TaskGraph
{
template <>
inline Scope<DeviceBuffer> Realize(const BufferDesc& desc)
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
template <>
inline Scope<DeviceTexture> Realize(const TextureDesc& desc)
{
    auto deviceTexture = DeviceTexture::Create(desc.width, desc.height, desc.pixelFormat, desc.usages, DeviceImageLayout::Undefined);
    if (!deviceTexture)
    {
        return nullptr;
    }
    return CreateScope<DeviceTexture>(std::move(deviceTexture));
}

} // namespace Aether::TaskGraph