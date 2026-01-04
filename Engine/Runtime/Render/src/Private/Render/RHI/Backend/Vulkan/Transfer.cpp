#include "Transfer.h"
namespace Aether::vk
{
void AsyncCopyBuffer(GraphicsCommandBuffer& commandBuffer, Buffer& src, Buffer& dst, size_t size, size_t srcOffset,
                     size_t dstOffset)
{
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer.GetHandle(), src.GetHandle(), dst.GetHandle(), 1, &copyRegion);
}
} // namespace Aether::vk