#pragma once
#include <Render/RenderApi.h>
#include "ResourceImpl.h"
#include <Core/Cache.h>
#include "RenderPass.h"
#include <vector>
namespace Aether::TaskGraph
{
class RenderPassResourcePool
{
public:
    void PushFrameBuffer(Scope<DeviceFrameBuffer>&& frameBuffer, const FrameBufferDesc& desc)
    {
        m_FrameBuffers.Put(desc, std::move(frameBuffer));
    }
    Scope<DeviceFrameBuffer> PopFrameBuffer(const FrameBufferDesc& desc)
    {
        auto frameBuffer = m_FrameBuffers.Pop(desc);
        if (!frameBuffer.has_value())
        {
            return nullptr;
        }
        return std::move(frameBuffer.value());
    }
    void PushRenderPass(Scope<DeviceRenderPass>&& renderPass, const RenderPassDesc& desc)
    {
        m_RenderPasses.Put(desc, std::move(renderPass));
    }
    Scope<DeviceRenderPass> PopRenderPass(const RenderPassDesc& desc)
    {
        auto renderPass = m_RenderPasses.Pop(desc);
        if (!renderPass.has_value())
        {
            return nullptr;
        }
        return std::move(renderPass.value());
    }
    void OnFrameBegin()
    {
        m_FrameBuffers.TrimToCapacity(m_Capacity);
        m_RenderPasses.TrimToCapacity(m_Capacity);
    }

private:
    size_t m_Capacity = 64;
    LRUCache<FrameBufferDesc, Scope<DeviceFrameBuffer>, Hash<FrameBufferDesc>> m_FrameBuffers;
    LRUCache<RenderPassDesc, Scope<DeviceRenderPass>, Hash<RenderPassDesc>> m_RenderPasses;
};

class CurrentFrameTransientResourcePool
{
};
class LRUTransientResourcePool
{
public:
private:

};
} // namespace Aether::TaskGraph