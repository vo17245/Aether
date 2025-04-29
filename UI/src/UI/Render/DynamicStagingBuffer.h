#pragma once
#include "Render/RenderApi.h"
#include "Render/RenderApi/DeviceBuffer.h"
namespace Aether::UI
{
    class DynamicStagingBuffer
    {
    public:
        DynamicStagingBuffer(size_t initSize)
        :m_Size(initSize)
        {
            m_Buffer=DeviceBuffer::CreateForStaging(initSize);
        }
        void SetData(uint8_t* data,size_t size)
        {
            if(size>m_Size)
            {
                Resize(size*1.5);
            }
            m_Buffer.SetData(data,size);
        }
        void Resize(size_t size)
        {
            m_Size=size;
            m_Buffer=DeviceBuffer::CreateForStaging(size);
        }
        DeviceBuffer& GetBuffer()
        {
            return m_Buffer;
        }
    private:
        DeviceBuffer m_Buffer;
        size_t m_Size;
    };  
}