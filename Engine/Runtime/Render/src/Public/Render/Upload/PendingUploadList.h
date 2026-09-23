#pragma once
#include <Render/RHI.h>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Aether
{
class PendingUploadList;
class TransientStagingBuffer
{
    friend class PendingUploadList;
public:
    explicit TransientStagingBuffer(rhi::StagingBuffer&& buffer) : m_Buffer(std::move(buffer)) {}
    ~TransientStagingBuffer() = default;
    rhi::StagingBuffer& GetBuffer() { return m_Buffer; }
    void SetData(size_t offset, std::span<const uint8_t> data) { m_Buffer.SetData(offset, data); }
private:
    std::optional<std::uint32_t> m_SubmittedFrameSlot;
    std::uint64_t m_SubmissionGeneration = 0;
    rhi::StagingBuffer m_Buffer;
};

enum class UploadTicketStatus : std::uint8_t { Pending, Recorded, Submitted, Completed, Failed, Cancelled };
using UploadTicketId = std::uint64_t;
using UploadTicketCallback = std::function<void(UploadTicketId, UploadTicketStatus,
                                                std::uint32_t, std::uint64_t)>;

class PendingUploadList
{
public:
    template<typename BufferType>
    requires (std::is_same_v<BufferType, rhi::VertexBuffer> || std::is_same_v<BufferType, rhi::IndexBuffer>)
    void UploadBuffer(std::span<const uint8_t> data, BufferType* dstBuffer, size_t dstOffset)
    {
        ValidateBufferRange(data.size(), dstBuffer, dstOffset);
        if (data.size() > m_MaxUnsubmittedBytes - m_UnsubmittedBytes)
            throw std::length_error("unsubmitted staging byte budget exceeded");
        auto command = CreateUploadBufferCommand(data, dstBuffer, dstOffset, 0, {});
        try { m_UploadBufferList.push_back(std::move(command)); }
        catch (...) { m_StagingBuffers.pop_back(); throw; }
        m_UnsubmittedBytes += data.size();
    }

    template<typename BufferType>
    requires (std::is_same_v<BufferType, rhi::VertexBuffer> || std::is_same_v<BufferType, rhi::IndexBuffer>)
    std::optional<UploadTicketId> TryUploadBufferTracked(std::span<const uint8_t> data, BufferType* dstBuffer,
                                      size_t dstOffset, UploadTicketCallback callback)
    {
        if (!callback) throw std::invalid_argument("tracked upload requires a completion callback");
        ValidateBufferRange(data.size(), dstBuffer, dstOffset);
        if (data.size() > m_MaxUnsubmittedBytes - m_UnsubmittedBytes) return std::nullopt;
        if (m_NextTicket == 0) throw std::overflow_error("upload ticket id exhausted");
        const auto ticket = m_NextTicket++;
        auto command = CreateUploadBufferCommand(data, dstBuffer, dstOffset, ticket, std::move(callback));
        try { m_UploadBufferList.push_back(std::move(command)); }
        catch (...) { m_StagingBuffers.pop_back(); throw; }
        m_UnsubmittedBytes += data.size();
        return ticket;
    }

    template<typename BufferType>
    requires (std::is_same_v<BufferType, rhi::VertexBuffer> || std::is_same_v<BufferType, rhi::IndexBuffer>)
    UploadTicketId UploadBufferTracked(std::span<const uint8_t> data, BufferType* dstBuffer,
                                       size_t dstOffset, UploadTicketCallback callback)
    {
        auto ticket = TryUploadBufferTracked(data, dstBuffer, dstOffset, std::move(callback));
        if (!ticket) throw std::length_error("unsubmitted staging byte budget exceeded");
        return *ticket;
    }

    // Copies CPU pixels into an owned staging buffer immediately. RecordCommand
    // emits layout transitions and the copy before rendering. Keep dst alive until
    // the submission completes. oldLayout describes its layout before this upload.
    void UploadTexture(std::span<const uint8_t> pixels, rhi::Texture2D* dst,
                       const rhi::TextureUploadRegion& region,
                       rhi::TextureLayout oldLayout = rhi::TextureLayout::ShaderReadOnly);
    // Called after the slot fence signals. The generation identifies the actual
    // queue submission that used this slot; CPU frame ids and command serials are not GPU fences.
    void OnFrameSlotCompleted(std::uint32_t frameSlot, std::uint64_t submissionGeneration);
    void OnFrameSlotCompleted(std::uint32_t frameSlot);
    void OnQueueSubmitted(std::uint32_t frameSlot, std::uint64_t submissionGeneration);
    void OnQueueSubmitFailed(std::uint32_t frameSlot);
    void RecordCommand(rhi::CommandList& commandBuffer, std::uint32_t frameSlot);
    bool HasPendingCommands() const noexcept
    {
        return !m_UploadBufferList.empty() || !m_UploadTextureList.empty();
    }
    std::size_t UnsubmittedStagingBytes() const noexcept { return m_UnsubmittedBytes; }
    std::size_t MaxUnsubmittedStagingBytes() const noexcept { return m_MaxUnsubmittedBytes; }
    void SetMaxUnsubmittedStagingBytes(std::size_t bytes);
    void ReleaseAll() noexcept;

private:
    struct UploadTextureCommand
    {
        TransientStagingBuffer* source = nullptr;
        rhi::Texture2D* destination = nullptr;
        rhi::TextureUploadRegion region;
        rhi::TextureLayout oldLayout;
        std::size_t size = 0;
    };
    using Buffer = std::variant<std::monostate, rhi::VertexBuffer*, rhi::IndexBuffer*>;
    struct UploadBufferCommand
    {
        TransientStagingBuffer* source = nullptr;
        Buffer destination;
        size_t sourceOffset = 0;
        size_t destinationOffset = 0;
        size_t size = 0;
        UploadTicketId ticket = 0;
        UploadTicketCallback callback;
    };
    struct RecordedCommands
    {
        std::vector<UploadBufferCommand> buffers;
        std::vector<UploadTextureCommand> textures;
    };
    struct InFlightTicket
    {
        UploadTicketId ticket = 0;
        UploadTicketCallback callback;
    };
    template <typename BufferType>
    static void ValidateBufferRange(std::size_t bytes, BufferType* destination, std::size_t offset)
    {
        if (!destination || !bytes || (bytes & 3u) || (offset & 3u)
            || offset > destination->GetSize() || bytes > destination->GetSize() - offset)
            throw std::invalid_argument("invalid or unaligned buffer upload range");
    }
    template <typename BufferType>
    UploadBufferCommand CreateUploadBufferCommand(std::span<const uint8_t> data, BufferType* dstBuffer,
                                                   size_t dstOffset, UploadTicketId ticket,
                                                   UploadTicketCallback callback)
    {
        if (!dstBuffer || data.empty()) throw std::invalid_argument("invalid buffer upload");
        auto stagingBuffer = AllocateStagingBuffer(data.size());
        try
        {
            if (!stagingBuffer->GetBuffer()) throw std::runtime_error("Failed to allocate buffer upload staging buffer");
            stagingBuffer->SetData(0, data);
        }
        catch (...)
        {
            m_StagingBuffers.pop_back();
            throw;
        }
        UploadBufferCommand command{};
        command.source = stagingBuffer;
        command.destination = dstBuffer;
        command.sourceOffset = 0;
        command.destinationOffset = dstOffset;
        command.size = data.size();
        command.ticket = ticket;
        command.callback = std::move(callback);
        return command;
    }
    TransientStagingBuffer* AllocateStagingBuffer(size_t size)
    {
        if (!size) throw std::invalid_argument("staging buffer size must be positive");
        m_StagingBuffers.push_back(CreateScope<TransientStagingBuffer>(rhi::StagingBuffer::Create(size)));
        return m_StagingBuffers.back().get();
    }
    static void Notify(const UploadBufferCommand& command, UploadTicketStatus status,
                       std::uint32_t slot, std::uint64_t generation) noexcept;

    UploadTicketId m_NextTicket = 1;
    std::size_t m_UnsubmittedBytes = 0;
    std::size_t m_MaxUnsubmittedBytes = 4ull * 1024 * 1024;
    std::map<std::uint32_t, std::uint64_t> m_LastSubmissionGeneration;
    std::map<std::uint32_t, RecordedCommands> m_Recorded;
    std::map<std::pair<std::uint32_t, std::uint64_t>, std::vector<InFlightTicket>> m_InFlight;
    std::vector<Scope<TransientStagingBuffer>> m_StagingBuffers;
    std::vector<UploadBufferCommand> m_UploadBufferList;
    std::vector<UploadTextureCommand> m_UploadTextureList;
};
} // namespace Aether
