#include "Render/Upload/PendingUploadList.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace Aether
{
void PendingUploadList::UploadTexture(std::span<const uint8_t> pixels, rhi::Texture2D* dst,
                                      const rhi::TextureUploadRegion& region, rhi::TextureLayout oldLayout)
{
    if (!dst || region.width == 0 || region.height == 0 ||
        region.x > dst->GetWidth() || region.width > dst->GetWidth() - region.x ||
        region.y > dst->GetHeight() || region.height > dst->GetHeight() - region.y ||
        !(dst->GetUsages() & static_cast<uint32_t>(rhi::TextureUsage::TransferDst)))
        throw std::invalid_argument("Invalid texture upload region, pixel size or destination usage");
    const auto pixelBytes = static_cast<std::size_t>(PixelFormatSize(dst->GetFormat()));
    if (!pixelBytes || region.width > std::numeric_limits<std::size_t>::max() / region.height
        || static_cast<std::size_t>(region.width) * region.height > std::numeric_limits<std::size_t>::max() / pixelBytes
        || pixels.size() != static_cast<std::size_t>(region.width) * region.height * pixelBytes)
        throw std::invalid_argument("Invalid texture upload pixel byte count");
    if (pixels.size() > m_MaxUnsubmittedBytes - m_UnsubmittedBytes)
        throw std::length_error("unsubmitted staging byte budget exceeded");
    auto* staging = AllocateStagingBuffer(pixels.size());
    try
    {
        if (!staging->GetBuffer()) throw std::runtime_error("Failed to allocate texture upload staging buffer");
        staging->SetData(0, pixels);
        m_UploadTextureList.push_back({staging, dst, region, oldLayout, pixels.size()});
    }
    catch (...)
    {
        m_StagingBuffers.pop_back();
        throw;
    }
    m_UnsubmittedBytes += pixels.size();
}

void PendingUploadList::Notify(const UploadBufferCommand& command, UploadTicketStatus status,
                               std::uint32_t slot, std::uint64_t generation) noexcept
{
    if (!command.ticket || !command.callback) return;
    try { command.callback(command.ticket, status, slot, generation); }
    catch (...) { /* Ticket callbacks are diagnostics/state notifications, never render failures. */ }
}

void PendingUploadList::RecordCommand(rhi::CommandList& commandBuffer, std::uint32_t frameSlot)
{
    RecordedCommands recorded;
    recorded.buffers.reserve(m_UploadBufferList.size());
    recorded.textures.reserve(m_UploadTextureList.size());
    for (auto& upload : m_UploadBufferList)
    {
        if (!upload.source || !upload.source->GetBuffer())
            throw std::runtime_error("buffer upload staging allocation is unavailable");
        std::visit([&](auto&& dstBuffer) {
            using T = std::decay_t<decltype(dstBuffer)>;
            if constexpr (std::is_same_v<T, rhi::VertexBuffer*>)
            {
                if (!dstBuffer) throw std::runtime_error("vertex upload destination is null");
                commandBuffer.UploadVertexBuffer(upload.source->m_Buffer, *dstBuffer, upload.size,
                                                 upload.sourceOffset, upload.destinationOffset);
            }
            else if constexpr (std::is_same_v<T, rhi::IndexBuffer*>)
            {
                if (!dstBuffer) throw std::runtime_error("index upload destination is null");
                commandBuffer.UploadIndexBuffer(upload.source->m_Buffer, *dstBuffer, upload.size,
                                                upload.sourceOffset, upload.destinationOffset);
            }
            else throw std::runtime_error("buffer upload destination type is invalid");
        }, upload.destination);
        recorded.buffers.push_back(upload);
    }
    for (auto& upload : m_UploadTextureList)
    {
        if (!upload.source || !upload.source->GetBuffer() || !upload.destination)
            throw std::runtime_error("texture upload resource is unavailable");
        commandBuffer.TextureLayoutTransition(*upload.destination, upload.oldLayout, rhi::TextureLayout::TransferDst);
        commandBuffer.UploadTexture(upload.source->m_Buffer, *upload.destination, upload.region);
        commandBuffer.TextureLayoutTransition(*upload.destination, rhi::TextureLayout::TransferDst,
                                              rhi::TextureLayout::ShaderReadOnly);
        recorded.textures.push_back(upload);
    }
    if (!recorded.buffers.empty() || !recorded.textures.empty())
    {
        if (m_Recorded.contains(frameSlot))
            throw std::logic_error("upload commands were recorded twice for one unsubmitted frame slot");
        auto [it, inserted] = m_Recorded.emplace(frameSlot, std::move(recorded));
        (void)inserted;
        for (const auto& upload : it->second.buffers)
            Notify(upload, UploadTicketStatus::Recorded, frameSlot, 0);
    }
    m_UploadBufferList.clear();
    m_UploadTextureList.clear();
}

void PendingUploadList::OnQueueSubmitted(std::uint32_t frameSlot, std::uint64_t generation)
{
    if (!generation) throw std::invalid_argument("submission generation must be non-zero");
    auto found = m_Recorded.find(frameSlot);
    m_LastSubmissionGeneration[frameSlot] = generation;
    if (found == m_Recorded.end()) return;
    auto commands = std::move(found->second);
    m_Recorded.erase(found);
    for (const auto& upload : commands.buffers) m_UnsubmittedBytes -= upload.size;
    for (const auto& upload : commands.textures) m_UnsubmittedBytes -= upload.size;
    auto& flight = m_InFlight[{frameSlot, generation}];
    for (auto& upload : commands.buffers)
    {
        upload.source->m_SubmittedFrameSlot = frameSlot;
        upload.source->m_SubmissionGeneration = generation;
        if (upload.ticket)
            flight.push_back({upload.ticket, upload.callback});
        Notify(upload, UploadTicketStatus::Submitted, frameSlot, generation);
    }
    for (auto& upload : commands.textures)
    {
        upload.source->m_SubmittedFrameSlot = frameSlot;
        upload.source->m_SubmissionGeneration = generation;
    }
}

void PendingUploadList::OnQueueSubmitFailed(std::uint32_t frameSlot)
{
    auto found = m_Recorded.find(frameSlot);
    if (found == m_Recorded.end()) return;
    auto commands = std::move(found->second);
    m_Recorded.erase(found);
    for (auto it = commands.buffers.rbegin(); it != commands.buffers.rend(); ++it)
    {
        Notify(*it, UploadTicketStatus::Pending, frameSlot, 0);
        m_UploadBufferList.insert(m_UploadBufferList.begin(), std::move(*it));
    }
    for (auto it = commands.textures.rbegin(); it != commands.textures.rend(); ++it)
        m_UploadTextureList.insert(m_UploadTextureList.begin(), std::move(*it));
}

void PendingUploadList::OnFrameSlotCompleted(std::uint32_t frameSlot, std::uint64_t generation)
{
    auto flight = m_InFlight.find({frameSlot, generation});
    if (flight != m_InFlight.end())
    {
        for (const auto& ticket : flight->second)
        {
            if (!ticket.callback) continue;
            try { ticket.callback(ticket.ticket, UploadTicketStatus::Completed, frameSlot, generation); }
            catch (...) { }
        }
        m_InFlight.erase(flight);
    }
    std::erase_if(m_StagingBuffers, [&](const Scope<TransientStagingBuffer>& buffer) {
        return buffer->m_SubmittedFrameSlot && *buffer->m_SubmittedFrameSlot == frameSlot
            && buffer->m_SubmissionGeneration == generation;
    });
}

void PendingUploadList::SetMaxUnsubmittedStagingBytes(std::size_t bytes)
{
    if (!bytes || bytes < m_UnsubmittedBytes)
        throw std::invalid_argument("staging budget must cover current unsubmitted uploads");
    m_MaxUnsubmittedBytes = bytes;
}

void PendingUploadList::OnFrameSlotCompleted(std::uint32_t frameSlot)
{
    const auto found = m_LastSubmissionGeneration.find(frameSlot);
    OnFrameSlotCompleted(frameSlot, found == m_LastSubmissionGeneration.end() ? 0 : found->second);
}

void PendingUploadList::ReleaseAll() noexcept
{
    for (const auto& upload : m_UploadBufferList)
        Notify(upload, UploadTicketStatus::Cancelled, 0, 0);
    for (const auto& [slot, commands] : m_Recorded)
        for (const auto& upload : commands.buffers)
            Notify(upload, UploadTicketStatus::Cancelled, slot, 0);
    for (const auto& [key, tickets] : m_InFlight)
        for (const auto& ticket : tickets)
            if (ticket.callback)
                try { ticket.callback(ticket.ticket, UploadTicketStatus::Cancelled, key.first, key.second); }
                catch (...) { }
    m_UploadBufferList.clear();
    m_UploadTextureList.clear();
    m_Recorded.clear();
    m_InFlight.clear();
    m_StagingBuffers.clear();
    m_UnsubmittedBytes = 0;
}
} // namespace Aether
