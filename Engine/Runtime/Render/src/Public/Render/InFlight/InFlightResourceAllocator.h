#pragma once

#include <Render/RHI.h>
#include <Render/RHI/Backend/Vulkan/DescriptorPool.h>
#include <Render/RHI/Backend/Vulkan/DescriptorSet.h>
#include <Render/RHI/Backend/Vulkan/DescriptorSetLayout.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace Aether
{
class Window;

class InFlightResourceAllocator;

namespace Detail
{
void ValidateFrameSlot(const InFlightResourceAllocator& allocator, std::uint32_t frameSlot);
}

class InFlightUniformBuffer
{
public:
    InFlightUniformBuffer() = default;
    InFlightUniformBuffer(const InFlightUniformBuffer&) = delete;
    InFlightUniformBuffer& operator=(const InFlightUniformBuffer&) = delete;
    InFlightUniformBuffer(InFlightUniformBuffer&&) noexcept = default;
    InFlightUniformBuffer& operator=(InFlightUniformBuffer&&) noexcept = default;

    std::uint32_t SlotCount() const noexcept { return static_cast<std::uint32_t>(m_Buffers.size()); }
    rhi::UniformBuffer& GetBuffer(std::uint32_t frameSlot);
    const rhi::UniformBuffer& GetBuffer(std::uint32_t frameSlot) const;
    rhi::UniformBuffer& GetBufferForImport(std::uint32_t frameSlot);
    const rhi::UniformBuffer& GetBufferForImport(std::uint32_t frameSlot) const;
    void SetData(std::uint32_t frameSlot, std::size_t offset, std::span<const std::uint8_t> data);

private:
    friend class InFlightResourceAllocator;
    InFlightUniformBuffer(InFlightResourceAllocator& allocator, std::size_t size);

    InFlightResourceAllocator* m_Allocator = nullptr;
    std::vector<std::unique_ptr<rhi::UniformBuffer>> m_Buffers;
};

class InFlightVertexBuffer
{
public:
    InFlightVertexBuffer() = default;
    InFlightVertexBuffer(const InFlightVertexBuffer&) = delete;
    InFlightVertexBuffer& operator=(const InFlightVertexBuffer&) = delete;
    InFlightVertexBuffer(InFlightVertexBuffer&&) noexcept = default;
    InFlightVertexBuffer& operator=(InFlightVertexBuffer&&) noexcept = default;

    std::uint32_t SlotCount() const noexcept { return static_cast<std::uint32_t>(m_Buffers.size()); }
    rhi::VertexBuffer& GetBuffer(std::uint32_t frameSlot);
    const rhi::VertexBuffer& GetBuffer(std::uint32_t frameSlot) const;
    rhi::VertexBuffer& GetBufferForImport(std::uint32_t frameSlot);
    const rhi::VertexBuffer& GetBufferForImport(std::uint32_t frameSlot) const;
    void SetData(std::uint32_t frameSlot, std::size_t offset, std::span<const std::uint8_t> data);

private:
    friend class InFlightResourceAllocator;
    InFlightVertexBuffer(InFlightResourceAllocator& allocator, std::size_t size);

    InFlightResourceAllocator* m_Allocator = nullptr;
    std::vector<std::unique_ptr<rhi::VertexBuffer>> m_Buffers;
};

class InFlightStorageBuffer
{
public:
    InFlightStorageBuffer() = default;
    InFlightStorageBuffer(const InFlightStorageBuffer&) = delete;
    InFlightStorageBuffer& operator=(const InFlightStorageBuffer&) = delete;
    InFlightStorageBuffer(InFlightStorageBuffer&&) noexcept = default;
    InFlightStorageBuffer& operator=(InFlightStorageBuffer&&) noexcept = default;

    std::uint32_t SlotCount() const noexcept { return static_cast<std::uint32_t>(m_Buffers.size()); }
    rhi::RWStructuredBuffer& GetBuffer(std::uint32_t frameSlot);
    const rhi::RWStructuredBuffer& GetBuffer(std::uint32_t frameSlot) const;
    rhi::RWStructuredBuffer& GetBufferForImport(std::uint32_t frameSlot);
    const rhi::RWStructuredBuffer& GetBufferForImport(std::uint32_t frameSlot) const;

private:
    friend class InFlightResourceAllocator;
    InFlightStorageBuffer(InFlightResourceAllocator& allocator, std::size_t size);

    InFlightResourceAllocator* m_Allocator = nullptr;
    std::vector<std::unique_ptr<rhi::RWStructuredBuffer>> m_Buffers;
};

class InFlightDescriptorSet
{
public:
    InFlightDescriptorSet() = default;
    InFlightDescriptorSet(const InFlightDescriptorSet&) = delete;
    InFlightDescriptorSet& operator=(const InFlightDescriptorSet&) = delete;
    InFlightDescriptorSet(InFlightDescriptorSet&&) noexcept = default;
    InFlightDescriptorSet& operator=(InFlightDescriptorSet&&) noexcept = default;

    std::uint32_t SlotCount() const noexcept { return static_cast<std::uint32_t>(m_Sets.size()); }
    vk::DescriptorSet& GetSet(std::uint32_t frameSlot);
    const vk::DescriptorSet& GetSet(std::uint32_t frameSlot) const;
    vk::DescriptorSet& GetSetForSetup(std::uint32_t frameSlot);
    const vk::DescriptorSet& GetSetForSetup(std::uint32_t frameSlot) const;

private:
    friend class InFlightResourceAllocator;
    // Renderer-facing descriptor sets own an explicit pool per frame slot.
    InFlightDescriptorSet(InFlightResourceAllocator& allocator,
                          const vk::DescriptorSetLayout& layout,
                          std::uint32_t uniformBufferCount,
                          std::uint32_t samplerCount,
                          std::uint32_t storageBufferCount);

    struct Slot
    {
        std::optional<vk::DescriptorPool> pool;
        std::optional<vk::DescriptorSet> set;
    };
    InFlightResourceAllocator* m_Allocator = nullptr;
    std::vector<Slot> m_Sets;
};

class InFlightResourceAllocator
{
public:
    explicit InFlightResourceAllocator(std::uint32_t frameSlotCount);
    InFlightResourceAllocator(const InFlightResourceAllocator&) = delete;
    InFlightResourceAllocator& operator=(const InFlightResourceAllocator&) = delete;

    std::uint32_t FrameSlotCount() const noexcept { return m_FrameSlotCount; }
    std::uint32_t CurrentFrameSlot() const noexcept { return m_CurrentFrameSlot; }

    InFlightUniformBuffer AllocateUniformBuffer(std::size_t size)
    {
        return InFlightUniformBuffer(*this, size);
    }
    InFlightVertexBuffer AllocateVertexBuffer(std::size_t size)
    {
        return InFlightVertexBuffer(*this, size);
    }
    InFlightStorageBuffer AllocateStorageBuffer(std::size_t size)
    {
        return InFlightStorageBuffer(*this, size);
    }
    InFlightDescriptorSet AllocateDescriptorSet(const vk::DescriptorSetLayout& layout,
                                                std::uint32_t uniformBufferCount,
                                                std::uint32_t samplerCount,
                                                std::uint32_t storageBufferCount)
    {
        return InFlightDescriptorSet(*this, layout, uniformBufferCount, samplerCount, storageBufferCount);
    }

private:
    friend class Window;
    void SetCurrentFrame(std::uint32_t frameSlot);

    std::uint32_t m_FrameSlotCount;
    std::uint32_t m_CurrentFrameSlot = 0;
};
} // namespace Aether
