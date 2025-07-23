#pragma once
#include <Render/RenderApi.h>
#include "Realize.h"
#include "ResourceImpl.h"
#include "TransientResourcePool.h"
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
    auto deviceTexture = DeviceTexture::Create(desc.width, desc.height, desc.pixelFormat, desc.usages, desc.layout);
    if (!deviceTexture)
    {
        return nullptr;
    }
    return CreateScope<DeviceTexture>(std::move(deviceTexture));
}
template<>
inline Scope<DeviceRenderPass> Realize(const RenderPassDesc& desc)
{
    DeviceRenderPassDesc d;
    d.colorAttachmentCount = desc.colorAttachmentCount;
    for (size_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        auto& dca= d.colorAttachments[i];
        dca.format=desc.colorAttachment[i].texture->GetDesc().pixelFormat;
    }
    if (desc.depthAttachment)
    {
        d.depthAttachment = DeviceAttachmentDesc{
            .loadOp = desc.depthAttachment->loadOp,
            .storeOp = desc.depthAttachment->storeOp,   
            .format = desc.depthAttachment->texture->GetDesc().pixelFormat,
            
        };
    }
    auto deviceRenderPass = DeviceRenderPass::Create(d);
    if (deviceRenderPass.Empty())
    {
        return nullptr;
    }
    return CreateScope<DeviceRenderPass>(std::move(deviceRenderPass));
}

} // namespace Aether::TaskGraph