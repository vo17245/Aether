#pragma once
#include <Render/RenderApi.h>

namespace Aether
{
    class InFlightBufferAllocator;
    class InFlightBuffer
    {
        friend class InFlightBufferAllocator;
    public:
        InFlightBuffer()=default;
        ~InFlightBuffer()=default;
        InFlightBuffer(const InFlightBuffer&) = delete;
        InFlightBuffer& operator=(const InFlightBuffer&) = delete;
        InFlightBuffer(InFlightBuffer&& other)=default;
        InFlightBuffer& operator=(InFlightBuffer&& other)=default;
    public:
        InFlightBuffer CreateUbo(size_t size)
        {
            InFlightBuffer res;
            for(size_t i=0;i<Render::Config::MaxFramesInFlight;++i)
            {
                res.m_Buffer[i]=DeviceBuffer::CreateForUniform(size);
                if(res.m_Buffer[i].Empty())
                {
                    return InFlightBuffer{};
                }
            }
            return res;
        }
        InFlightBuffer CreateSsbo(size_t size)
        {
            InFlightBuffer res;
            for(size_t i=0;i<Render::Config::MaxFramesInFlight;++i)
            {
                res.m_Buffer[i]=DeviceBuffer::CreateForSSBO(size);
                if(res.m_Buffer[i].Empty())
                {
                    return InFlightBuffer{};
                }
            }
            return res;
        }
        DeviceBuffer& GetBuffer();
        const DeviceBuffer& GetBuffer()const;
    private:
        DeviceBuffer m_Buffer[Render::Config::InFlightFrameResourceSlots];
        InFlightBufferAllocator* m_Allocator=nullptr;
    };
    class InFlightBufferAllocator
    {
    public:
        void SetCurrentFrame(uint32_t frame)
        {
            m_CurrentFrameIndex=frame;
        }
        uint32_t GetCurrentFrame()const
        {
            return m_CurrentFrameIndex;
        }
        InFlightBuffer AllocateUbo(size_t size)
        {
            auto buffer=InFlightBuffer{}.CreateUbo(size);
            buffer.m_Allocator=this;
            return buffer;
        }
        InFlightBuffer AllocateSsbo(size_t size)
        {
            auto buffer=InFlightBuffer{}.CreateSsbo(size);
            buffer.m_Allocator=this;
            return buffer;
        }
    private:
        uint32_t m_CurrentFrameIndex=0;
    };
}