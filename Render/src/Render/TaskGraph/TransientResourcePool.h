#pragma once
#include <Render/RenderApi.h>
#include "ResourceImpl.h"
#include <Core/Cache.h>
#include "RenderPass.h"
namespace Aether::TaskGraph 
{
    class TransientResourcePool
    {
    public:
        TransientResourcePool(size_t frameBufferCacheSize = 64, size_t renderPassCacheSize = 64)
            : m_FrameBuffers(frameBufferCacheSize), m_RenderPasses(renderPassCacheSize) {}
        void PushFrameBuffer(Scope<DeviceFrameBuffer>&& frameBuffer,const FrameBufferDesc& desc)
        {
            m_FrameBuffers.Put(desc, std::move(frameBuffer));
        }
        Scope<DeviceFrameBuffer> PopFrameBuffer(const FrameBufferDesc& desc)
        {
            auto frameBuffer = m_FrameBuffers.Pop(desc);
            if(!frameBuffer.has_value())
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
        
    private:

        LRUCache<FrameBufferDesc, Scope<DeviceFrameBuffer>,Hash<FrameBufferDesc>> m_FrameBuffers;
        LRUCache<RenderPassDesc, Scope<DeviceRenderPass>,Hash<RenderPassDesc>> m_RenderPasses;
    };
}