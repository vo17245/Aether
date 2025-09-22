#include "PendingUploadList.h"
namespace Aether
{
PendingUploadList::UploadBufferCommand
PendingUploadList::CreateUploadBufferCommand(std::span<const uint8_t> data, DeviceBuffer& dstBuffer, size_t dstOffset,uint32_t ttl)
{
    auto stagingBuffer = AllocateStagingBuffer(data.size());
    stagingBuffer->SetData(data);
    stagingBuffer->m_TTL=ttl;
    auto command = UploadBufferCommand{};
    command.source = stagingBuffer;
    command.destination = &dstBuffer;
    command.sourceOffset = 0;
    command.destinationOffset = dstOffset;
    command.size = data.size();
    return command;
}
void PendingUploadList::UploadBuffer(std::span<const uint8_t> data, DeviceBuffer& dstBuffer, size_t dstOffset)
{
    m_UploadBufferList.push_back(CreateUploadBufferCommand(data, dstBuffer, dstOffset,2));
}
void PendingUploadList::UploadInFlightBuffer(std::span<const uint8_t> data, DeviceBuffer& dstBuffer, size_t dstOffset)
{
    m_UploadInFlightBufferList.push_back(CreateUploadBufferCommand(data, dstBuffer, dstOffset,Render::Config::MaxFramesInFlight+1));
}
} // namespace Aether