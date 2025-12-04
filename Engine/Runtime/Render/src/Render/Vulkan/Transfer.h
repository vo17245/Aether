#pragma once
#include "GraphicsCommandBuffer.h"
#include "Buffer.h"
namespace Aether::vk
{
    void AsyncCopyBuffer(GraphicsCommandBuffer& commandBuffer,Buffer& src,Buffer& dst,size_t size,size_t srcOffset,size_t dstOffset);
}