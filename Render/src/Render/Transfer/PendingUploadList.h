#pragma once
#include <Render/RenderApi.h>
namespace Aether
{
class PendingUploadList;
class TransientStagingBuffer
{
    friend class PendingUploadList;

public:
    TransientStagingBuffer(int ttl, DeviceBuffer&& buffer) : m_TTL(ttl), m_Buffer(std::move(buffer))
    {
    }
    ~TransientStagingBuffer()
    {
    }
    void SetData(std::span<const uint8_t> data)
    {
        m_Buffer.SetData(data);
    }

private:
    int m_TTL = 0;
    DeviceBuffer m_Buffer;
};
class PendingUploadList
{
public:
    
    void UploadBuffer(std::span<const uint8_t> data,DeviceBuffer& dstBuffer,size_t dstOffset)
    {
        auto stagingBuffer = AllocateStagingBuffer(data.size());
        stagingBuffer->SetData(data);
        m_UploadBufferList.push_back({stagingBuffer, &dstBuffer, 0, dstOffset, data.size()});
    }
    void OnUpdate()
    {
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
    void RecordCommand(DeviceCommandBuffer& commandBuffer)
    {
        for (auto& upload : m_UploadBufferList)
        {
            commandBuffer.CopyBuffer(upload.source->m_Buffer, *upload.destination, upload.size, upload.sourceOffset,
                                     upload.destinationOffset);
        }
        m_UploadBufferList.clear();
    }
    bool AnyPending() const
    {
        return !m_UploadBufferList.empty();
    }
private:
    struct UploadBufferCommand
    {
        TransientStagingBuffer* source;
        DeviceBuffer* destination;
        size_t sourceOffset;
        size_t destinationOffset;
        size_t size;
    };
    TransientStagingBuffer* AllocateStagingBuffer(size_t size)
    {
        m_StagingBuffers.push_back(CreateScope<TransientStagingBuffer>(Render::Config::MaxFramesInFlight,
                                                                       DeviceBuffer::CreateForStaging(size)));
        return m_StagingBuffers.back().get();
    }
private:
    std::vector<Scope<TransientStagingBuffer>> m_StagingBuffers;
    std::vector<UploadBufferCommand> m_UploadBufferList;
};
} // namespace Aether