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
    template<typename BufferType>
    requires rhi::IsBufferType<BufferType>
    void UploadBuffer(std::span<const uint8_t> data, rhi::VertexBuffer* dstBuffer, size_t dstOffset)
    {
        m_UploadBufferList.push_back(
        CreateUploadBufferCommand(data, dstBuffer, dstOffset, Render::Config::MaxFramesInFlight));   
    }

public:
    void OnUpdate(bool minilized);
    void RecordCommand(rhi::CommandList& commandBuffer);

private:
    using Buffer = std::variant<std::monostate, rhi::VertexBuffer*, rhi::IndexBuffer*>;
    struct UploadBufferCommand
    {
        TransientStagingBuffer* source = nullptr;
        Buffer destination;
        size_t sourceOffset = 0;
        size_t destinationOffset = 0;
        size_t size = 0;
    };
    template <typename BufferType>
    UploadBufferCommand CreateUploadBufferCommand(std::span<const uint8_t> data, BufferType* dstBuffer,
                                                  size_t dstOffset, uint32_t ttl)
    {
        auto stagingBuffer = AllocateStagingBuffer(data.size());
        stagingBuffer->SetData(0, data);
        stagingBuffer->m_TTL = ttl;
        auto command = UploadBufferCommand{};
        command.source = stagingBuffer;
        command.destination = dstBuffer;
        command.sourceOffset = 0;
        command.destinationOffset = dstOffset;
        command.size = data.size();
        return command;
    }

    TransientStagingBuffer* AllocateStagingBuffer(size_t size)
    {
        m_StagingBuffers.push_back(
            CreateScope<TransientStagingBuffer>(Render::Config::MaxFramesInFlight, rhi::StagingBuffer::Create(size)));
        return m_StagingBuffers.back().get();
    }

private:
    std::vector<Scope<TransientStagingBuffer>> m_StagingBuffers;
    std::vector<UploadBufferCommand> m_UploadBufferList;
};
} // namespace Aether