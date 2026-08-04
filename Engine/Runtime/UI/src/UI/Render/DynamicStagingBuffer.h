#pragma once
#include <Render/RHI.h>

namespace Aether::UI
{
class DynamicStagingBuffer
{
public:
    DynamicStagingBuffer(size_t initSize) : m_Size(initSize)
    {
        m_Buffer = rhi::StagingBuffer::Create(initSize);
    }
    void SetData(uint8_t* data, size_t size)
    {
        if (size > m_Size)
        {
            Resize(static_cast<size_t>(size * 1.5));
        }
        m_Buffer.SetData(0, std::span<const uint8_t>(data, size));
    }
    void Resize(size_t size)
    {
        m_Size = size;
        m_Buffer = rhi::StagingBuffer::Create(size);
    }
    rhi::StagingBuffer& GetBuffer()
    {
        return m_Buffer;
    }

private:
    rhi::StagingBuffer m_Buffer;
    size_t m_Size;
};
} // namespace Aether::UI
