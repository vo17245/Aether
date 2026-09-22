#pragma once
#include <Render/RHI.h>
#include <optional>
namespace Aether
{
class PendingUploadList;
class TransientStagingBuffer
{
    friend class PendingUploadList;

public:
    explicit TransientStagingBuffer(rhi::StagingBuffer&& buffer) : m_Buffer(std::move(buffer))
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
    std::optional<std::uint32_t> m_SubmittedFrameSlot;
    rhi::StagingBuffer m_Buffer;
};
class PendingUploadList
{
public:
    template<typename BufferType>
    requires (std::is_same_v<BufferType, rhi::VertexBuffer> || std::is_same_v<BufferType, rhi::IndexBuffer>)
    void UploadBuffer(std::span<const uint8_t> data, BufferType* dstBuffer, size_t dstOffset)
    {
        m_UploadBufferList.push_back(
        CreateUploadBufferCommand(data, dstBuffer, dstOffset));
    }

public:
    // Copies CPU pixels into an owned staging buffer immediately. RecordCommand
    // emits layout transitions and the copy before rendering. Keep dst alive until
    // the submission completes. oldLayout describes its layout before this upload.
    void UploadTexture(std::span<const uint8_t> pixels, rhi::Texture2D* dst,
                       const rhi::TextureUploadRegion& region,
                       rhi::TextureLayout oldLayout = rhi::TextureLayout::ShaderReadOnly);
    // Release only staging associated with a frame slot whose fence completed.
    void OnFrameSlotCompleted(std::uint32_t frameSlot);
    void RecordCommand(rhi::CommandList& commandBuffer, std::uint32_t frameSlot);
    bool HasPendingCommands() const noexcept
    {
        return !m_UploadBufferList.empty() || !m_UploadTextureList.empty();
    }
    // Call only after the device/submissions using these staging buffers are
    // idle. This makes their physical destruction occur on the render owner.
    void ReleaseAll() noexcept
    {
        m_UploadBufferList.clear();
        m_UploadTextureList.clear();
        m_StagingBuffers.clear();
    }

private:
    struct UploadTextureCommand
    {
        TransientStagingBuffer* source = nullptr;
        rhi::Texture2D* destination = nullptr;
        rhi::TextureUploadRegion region;
        rhi::TextureLayout oldLayout;
    };
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
                                                  size_t dstOffset)
    {
        auto stagingBuffer = AllocateStagingBuffer(data.size());
        stagingBuffer->SetData(0, data);
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
            CreateScope<TransientStagingBuffer>(rhi::StagingBuffer::Create(size)));
        return m_StagingBuffers.back().get();
    }

private:
    std::vector<Scope<TransientStagingBuffer>> m_StagingBuffers;
    std::vector<UploadBufferCommand> m_UploadBufferList;
    std::vector<UploadTextureCommand> m_UploadTextureList;
};
} // namespace Aether
