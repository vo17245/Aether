#pragma once

#include "GraphicsCommandPool.h"
#include "vulkan/vulkan_core.h"
#include <cassert>
#include <cstdint>
#include <optional>
#include <type_traits>
#include "vma/vk_mem_alloc.h"
#include <cstring>

namespace Aether {
class Application;
namespace vk {

/**
 *@brief
 *   BeginMap 和 EndMap 成对使用
 *   SetData 单独使用
 *   是两种独立的更新数据的操作
 */
class Buffer
{
public:
    enum class Usage : int32_t
    {
        None,
        Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        Indirect = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    };

    enum class Property : int32_t
    {
        None,
        HostVisible = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        DeviceLocal = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        Staging = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        // Storage=VMA_MEMORY_USAGE_CPU_TO_GPU,
        //  HostCoherent = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        //  HostCached = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        //  LazilyAllocated = VMA_ALLOCATION_CREATE_MAPPED_BIT
    };
    template <typename... Ts>
    static constexpr Property PackProperties(Ts... args)
    {
        return (Property)(static_cast<std::underlying_type_t<Property>>(args) | ...);
    }
    template <typename... Ts>
    static constexpr Usage PackUsages(Ts... args)
    {
        return (Usage)(static_cast<std::underlying_type_t<Usage>>(args) | ...);
    }
    static bool SyncCopy(const GraphicsCommandPool& commandPool, Buffer& src, Buffer& dst, size_t size);
    static bool SyncCopy(const GraphicsCommandPool& commandPool, Buffer& src, Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    static bool SyncCopy(Buffer& src, Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
    
public:
    static std::optional<Buffer> Create(size_t size, Usage usage, Property properties);
    static std::optional<Buffer> CreateForStaging(size_t size)
    {
        return Create(size,
                      Buffer::Usage::TransferSrc,
                      Buffer::PackProperties(Buffer::Property::Staging));
    }
    static std::optional<Buffer> CreateForVertex(size_t size)
    {
        return Create(size,
                      Buffer::PackUsages(Buffer::Usage::Vertex, Buffer::Usage::TransferDst),
                      Buffer::PackProperties(Buffer::Property::DeviceLocal, Buffer::Property::HostVisible));
    }
    static std::optional<Buffer> CreateForIndex(size_t size)
    {
        return Create(size,
                      Buffer::PackUsages(Buffer::Usage::Index, Buffer::Usage::TransferDst),
                      Buffer::PackProperties(Buffer::Property::DeviceLocal, Buffer::Property::HostVisible));
    }
    static std::optional<Buffer> CreateForSSBO(size_t size);
    static std::optional<Buffer> CreateForUBO(size_t size);
    uint32_t GetSize() const;
    VkBuffer GetHandle() const;
    Usage GetUsage() const;
    void SetData(const uint8_t* data, size_t size);

    Buffer()
    {
        memset(&m_AllocInfo, 0, sizeof(VmaAllocationInfo));
    }
    operator bool() const
    {
        return m_Handle != VK_NULL_HANDLE;
    }

    template <typename T = char>
    T* BeginMap()
    {
        return static_cast<T*>(BeginMapImpl());
    }
    void EndMap()
    {
        EndMapImpl();
    }

public:
    ~Buffer();
    Buffer(const Buffer& other) = delete;
    Buffer& operator=(const Buffer& other) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

private:
    Buffer(VkBuffer handle, VmaAllocation allocation, size_t size, Usage usage, Property properties, VmaAllocationInfo allocInfo);
    void* BeginMapImpl();
    void EndMapImpl();

private:
    bool IsPersistenlyMapped()
    {
        return static_cast<std::underlying_type_t<Property>>(m_Properties) & VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

private:
    VkBuffer m_Handle = VK_NULL_HANDLE;
    size_t m_Size = 0;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    Usage m_Usage = Usage::None;
    Property m_Properties = Property::None;
    VmaAllocationInfo m_AllocInfo;
};
} // namespace vk
} // namespace Aether