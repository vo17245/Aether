#include "Render/Upload/PendingUploadList.h"
namespace Aether
{
PendingUploadList::UploadVertexBufferCommand
PendingUploadList::CreateUploadVertexBufferCommand(std::span<const uint8_t> data, rhi::VertexBuffer& dstBuffer, size_t dstOffset,uint32_t ttl)
{
    auto stagingBuffer = AllocateStagingBuffer(data.size());
    stagingBuffer->SetData(0, data);
    stagingBuffer->m_TTL=ttl;
    auto command = UploadVertexBufferCommand{};
    command.source = stagingBuffer;
    command.destination = &dstBuffer;
    command.sourceOffset = 0;
    command.destinationOffset = dstOffset;
    command.size = data.size();
    return command;
}
void PendingUploadList::UploadVertexBuffer(std::span<const uint8_t> data, rhi::VertexBuffer& dstBuffer, size_t dstOffset)
{
    m_UploadVertexBufferList.push_back(CreateUploadVertexBufferCommand(data, dstBuffer, dstOffset,2));
}

} // namespace Aether