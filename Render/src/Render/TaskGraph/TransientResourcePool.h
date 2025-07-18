#pragma once
#include <Render/RenderApi.h>
#include "ResourceImpl.h"
namespace Aether::TaskGraph 
{
    class TransientResourcePool
    {
    public:
        void PushFrameBufferf(Scope<DeviceFrameBuffer>&& frameBuffer,const FrameBufferDesc& desc)
        {
            m_FrameBuffers.emplace(desc, std::move(frameBuffer));
        }
        Scope<DeviceFrameBuffer> GetFrameBuffer(const FrameBufferDesc& desc)
        {
            auto it = m_FrameBuffers.find(desc);
            if (it != m_FrameBuffers.end())
            {
                return std::move(it->second);
            }
            return nullptr;
        }
    private:
        std::unordered_map<FrameBufferDesc, Scope<DeviceFrameBuffer>> m_FrameBuffers;
    };
}