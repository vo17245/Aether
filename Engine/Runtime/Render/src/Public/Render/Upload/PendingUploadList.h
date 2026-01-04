#pragma once
#include <Render/RHI.h>
namespace Aether
{
class PendingUploadList;
class TransientStagingBuffer
{
    friend class PendingUploadList;

public:
    TransientStagingBuffer(int ttl, rhi::StagingBuffer&& buffer) : m_TTL(ttl), m_Buffer(std::move(buffer))
    {
    }
    ~TransientStagingBuffer()
    {
    }
    rhi::StagingBuffer& GetBuffer()
    {
        return m_Buffer;
    }
    void SetData(size_t offset, std::span<const uint8_t> data)
    {
        m_Buffer.SetData(offset, data);
    }

private:
    int m_TTL = 0;
    rhi::StagingBuffer m_Buffer;
};
class PendingUploadList
{
public:
    void UploadVertexBuffer(std::span<const uint8_t> data, rhi::VertexBuffer& dstBuffer, size_t dstOffset);
public:
    void OnUpdate(bool minilized)
    {
        if (minilized)
        {
            return;
        }
        for (auto iter = m_StagingBuffers.begin(); iter != m_StagingBuffers.end();)
        {
            auto& buffer = *iter;
            buffer->m_TTL--;
            if (buffer->m_TTL <= 0)
            {
                iter = m_StagingBuffers.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    void RecordCommand(rhi::CommandList& commandBuffer)
    {
        for (auto& upload : m_UploadVertexBufferList)
        {
            commandBuffer.CopyBuffer(upload.source->m_Buffer, *upload.destination, upload.size, upload.sourceOffset,
                                     upload.destinationOffset);
        }
        m_UploadVertexBufferList.clear();
    }

private:
    struct UploadVertexBufferCommand
    {
        TransientStagingBuffer* source = nullptr;
        rhi::VertexBuffer* destination = nullptr;
        size_t sourceOffset = 0;
        size_t destinationOffset = 0;
        size_t size = 0;
    };
    UploadVertexBufferCommand CreateUploadVertexBufferCommand(std::span<const uint8_t> data, rhi::VertexBuffer& dstBuffer,
                                                  size_t dstOffset, uint32_t ttl);

    TransientStagingBuffer* AllocateStagingBuffer(size_t size)
    {
        m_StagingBuffers.push_back(CreateScope<TransientStagingBuffer>(Render::Config::MaxFramesInFlight,
                                                                       rhi::StagingBuffer::Create(size)));
        return m_StagingBuffers.back().get();
    }

private:
    std::vector<Scope<TransientStagingBuffer>> m_StagingBuffers;
    std::vector<UploadVertexBufferCommand> m_UploadVertexBufferList;
};
} // namespace Aether