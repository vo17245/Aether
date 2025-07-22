#pragma once
#include <Render/RenderApi.h>
#include "ResourceImpl.h"
#include <Core/Cache.h>
namespace Aether::TaskGraph 
{
    class TransientResourcePool
    {
    public:
        TransientResourcePool(size_t frameBufferCacheSize = 64, size_t renderPassCacheSize = 64)
            : m_FrameBuffers(frameBufferCacheSize), m_RenderPasses(renderPassCacheSize) {}
        void PushFrameBufferf(Scope<DeviceFrameBuffer>&& frameBuffer,const FrameBufferDesc& desc)
        {
            m_FrameBuffers.Put(desc, std::move(frameBuffer));
        }
        Scope<DeviceFrameBuffer> PopFrameBuffer(const FrameBufferDesc& desc)
        {
            auto frameBuffer = m_FrameBuffers.Get(desc);
            if (frameBuffer.has_value())
            {
                auto res= std::move(frameBuffer.value());
                m_FrameBuffers.Erase(desc);
                return std::move(res);
            }
            return nullptr;
        }
        void PushRenderPass(Scope<DeviceRenderPass>&& renderPass, const RenderPassDesc& desc)
        {
            m_RenderPasses.Put(desc, std::move(renderPass));
        }
        Scope<DeviceRenderPass> PopRenderPass(const RenderPassDesc& desc)
        {
            auto renderPass = m_RenderPasses.Get(desc);
            if (renderPass.has_value())
            {
                auto res = std::move(renderPass.value());
                m_RenderPasses.Erase(desc);
                return std::move(res);
            }
            return nullptr;
        }
        
    private:

        LRUCache<FrameBufferDesc, Scope<DeviceFrameBuffer>,Hash<FrameBufferDesc>> m_FrameBuffers;
        LRUCache<RenderPassDesc, Scope<DeviceRenderPass>,Hash<RenderPassDesc>> m_RenderPasses;
    };
}