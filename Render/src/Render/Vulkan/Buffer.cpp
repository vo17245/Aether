#include "Buffer.h"
#include "GraphicsCommandBuffer.h"
#include "Fence.h"

#include "Allocator.h"
#include "Buffer.h"
#include "GlobalRenderContext.h"
namespace Aether {
namespace vk {

struct CreateHandleResult
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocInfo;
    bool isOk;
};

static CreateHandleResult CreateHandle(uint32_t size, Buffer::Usage usage, Buffer::Property properties)
{
    VkBuffer buffer;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = static_cast<std::underlying_type_t<Buffer::Usage>>(usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = static_cast<std::underlying_type_t<Buffer::Property>>(properties);
    VmaAllocation allocation;
    VmaAllocationInfo allocInfo;

    vmaCreateBuffer(Allocator::Get(), &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo);
    if (buffer == VK_NULL_HANDLE && allocation == VK_NULL_HANDLE)
    {
        return {VK_NULL_HANDLE, VK_NULL_HANDLE, allocInfo, false};
    }
    else if (buffer == VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(Allocator::Get(), buffer, allocation);
        return {VK_NULL_HANDLE, VK_NULL_HANDLE, allocInfo, false};
    }
    else if (buffer != VK_NULL_HANDLE && allocation == VK_NULL_HANDLE)
    {
        vkDestroyBuffer(GRC::GetDevice(), buffer, nullptr);
        return {VK_NULL_HANDLE, VK_NULL_HANDLE, allocInfo, false};
    }
    else
    {
        return {buffer, allocation, allocInfo, true};
    }
}
bool Buffer::SyncCopy(const GraphicsCommandPool& commandPool, Buffer& src, Buffer& dst, size_t size)
{
    return SyncCopy(commandPool, src, dst, size, 0, 0);
}
bool Buffer::SyncCopy(const GraphicsCommandPool& commandPool, Buffer& src, Buffer& dst, size_t size,
                      size_t srcOffset, size_t dstOffset)
{
    // TODO: error handle
    auto fenceOpt = Fence::Create();
    auto fence = std::move(fenceOpt.value());
    auto commandBuffer = GraphicsCommandBuffer::Create(commandPool);
    commandBuffer->BeginSingleTime();
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer->GetHandle(), src.GetHandle(), dst.GetHandle(), 1, &copyRegion);
    //// insert memory barrier
    // VkBufferMemoryBarrier barrier{};
    // barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    // barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    // barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // barrier.buffer = dst.GetHandle();
    // barrier.offset = dstOffset;
    // barrier.size = size;
    // vkCmdPipelineBarrier(commandBuffer->GetHandle(),
    //                      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    //                      0,
    //                      0, nullptr,
    //                      1, &barrier,
    //                      0, nullptr);

    commandBuffer->End();
    commandBuffer->BeginSubmit()
        .Fence(fence)
        .EndSubmit();
    fence.Wait();
    return true;
}

std::optional<Buffer> Buffer::Create(size_t size, Usage usage, Property properties)
{
    auto handleRes = CreateHandle(size, usage, properties);
    if (handleRes.isOk == false)
    {
        return std::nullopt;
    }

    auto res = Buffer(handleRes.buffer, handleRes.allocation, size, usage, properties, handleRes.allocInfo);

    return res;
}

uint32_t Buffer::GetSize() const
{
    return m_Size;
}
VkBuffer Buffer::GetHandle() const
{
    return m_Handle;
}

Buffer::Usage Buffer::GetUsage() const
{
    return m_Usage;
}
void Buffer::SetData(const uint8_t* data, size_t size)
{
    if (IsPersistenlyMapped())
    {
        memcpy(m_AllocInfo.pMappedData, data, size);
    }
    else
    {
        void* mappedData;
        vmaMapMemory(Allocator::Get(), m_Allocation, &mappedData);
        memcpy(mappedData, data, size);
        vmaUnmapMemory(Allocator::Get(), m_Allocation);
    }
}

Buffer::~Buffer()
{
    if (m_Handle != VK_NULL_HANDLE)
        vmaDestroyBuffer(Allocator::Get(), m_Handle, m_Allocation);
}

Buffer::Buffer(Buffer&& other) noexcept
{
    m_Handle = other.m_Handle;
    m_Size = other.m_Size;
    m_Allocation = other.m_Allocation;
    m_Usage = other.m_Usage;
    m_AllocInfo = other.m_AllocInfo;
    other.m_Handle = VK_NULL_HANDLE;
    other.m_Allocation = VK_NULL_HANDLE;
}
Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other)
    {
        if (m_Handle != VK_NULL_HANDLE)
            vmaDestroyBuffer(Allocator::Get(), m_Handle, m_Allocation);

        m_Handle = other.m_Handle;
        m_Size = other.m_Size;
        m_Allocation = other.m_Allocation;
        m_Usage = other.m_Usage;
        m_AllocInfo = other.m_AllocInfo;
        other.m_Handle = VK_NULL_HANDLE;
        other.m_Allocation = VK_NULL_HANDLE;
    }
    return *this;
}

Buffer::Buffer(VkBuffer handle, VmaAllocation allocation, size_t size, Usage usage, Property properties, VmaAllocationInfo allocInfo) :
    m_Handle(handle), m_Size(size), m_Allocation(allocation), m_Usage(usage), m_Properties(properties), m_AllocInfo(allocInfo)
{
}

void* Buffer::BeginMapImpl()
{
    if (IsPersistenlyMapped())
    {
        return m_AllocInfo.pMappedData;
    }
    else
    {
        void* mappedData;
        vmaMapMemory(Allocator::Get(), m_Allocation, &mappedData);
        return mappedData;
    }
}
void Buffer::EndMapImpl()
{
    if (!IsPersistenlyMapped())
    {
        vmaUnmapMemory(Allocator::Get(), m_Allocation);
    }
}
std::optional<Buffer> Buffer::CreateForSSBO(size_t size)
{
    return Create(size,
                  Buffer::PackUsages(Buffer::Usage::Storage, Buffer::Usage::TransferDst),
                  Buffer::PackProperties(Buffer::Property::DeviceLocal, Buffer::Property::HostVisible));
}
std::optional<Buffer> Buffer::CreateForUBO(size_t size)
{
    return Create(size,
                  Buffer::PackUsages(Buffer::Usage::Uniform, Buffer::Usage::TransferDst),
                  Buffer::PackProperties(Buffer::Property::DeviceLocal, Buffer::Property::HostVisible));
}
}
} // namespace Aether::vk