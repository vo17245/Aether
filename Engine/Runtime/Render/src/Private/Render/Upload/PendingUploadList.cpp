#include "Render/Upload/PendingUploadList.h"
#include <stdexcept>
namespace Aether
{
void PendingUploadList::UploadTexture(std::span<const uint8_t> pixels, rhi::Texture2D* dst,
                                      const rhi::TextureUploadRegion& region, rhi::TextureLayout oldLayout)
{
    if (!dst || region.width == 0 || region.height == 0 ||
        region.x > dst->GetWidth() || region.width > dst->GetWidth() - region.x ||
        region.y > dst->GetHeight() || region.height > dst->GetHeight() - region.y ||
        pixels.size() != static_cast<size_t>(region.width) * region.height * PixelFormatSize(dst->GetFormat()) ||
        !(dst->GetUsages() & static_cast<uint32_t>(rhi::TextureUsage::TransferDst)))
        throw std::invalid_argument("Invalid texture upload region, pixel size or destination usage");
    auto* staging = AllocateStagingBuffer(pixels.size());
    if (!staging->GetBuffer())
        throw std::runtime_error("Failed to allocate texture upload staging buffer");
    staging->SetData(0, pixels);
    m_UploadTextureList.push_back({staging, dst, region, oldLayout});
}

void PendingUploadList::RecordCommand(rhi::CommandList& commandBuffer)
{
    for (auto& upload : m_UploadBufferList)
    {
        upload.source->m_Recorded = true;
        std::visit(
            [&](auto&& dstBuffer) {
                using T = std::decay_t<decltype(dstBuffer)>;
                if constexpr (std::is_same_v<T, rhi::VertexBuffer*>)
                {
                    commandBuffer.UploadVertexBuffer(upload.source->m_Buffer, *dstBuffer, upload.size,
                                                     upload.sourceOffset, upload.destinationOffset);
                }
                else if constexpr (std::is_same_v<T, rhi::IndexBuffer*>)
                {
                    commandBuffer.UploadIndexBuffer(upload.source->m_Buffer, *dstBuffer, upload.size,
                                                    upload.sourceOffset, upload.destinationOffset);
                }
            },
            upload.destination);
    }
    m_UploadBufferList.clear();
    for (auto& upload : m_UploadTextureList)
    {
        commandBuffer.TextureLayoutTransition(*upload.destination, upload.oldLayout, rhi::TextureLayout::TransferDst);
        commandBuffer.UploadTexture(upload.source->m_Buffer, *upload.destination, upload.region);
        commandBuffer.TextureLayoutTransition(*upload.destination, rhi::TextureLayout::TransferDst,
                                              rhi::TextureLayout::ShaderReadOnly);
        upload.source->m_Recorded = true;
    }
    m_UploadTextureList.clear();
}
void PendingUploadList::OnUpdate(bool minilized)
{
    if (minilized)
    {
        return;
    }
    for (auto iter = m_StagingBuffers.begin(); iter != m_StagingBuffers.end();)
    {
        auto& buffer = *iter;
        if (!buffer->m_Recorded)
        {
            ++iter;
            continue;
        }
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

} // namespace Aether