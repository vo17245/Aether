#include <Render/InFlight/InFlightResourceAllocator.h>

namespace Aether
{
namespace
{
template <typename T>
std::vector<std::unique_ptr<T>> CreateBuffers(InFlightResourceAllocator& allocator,
                                              std::size_t size,
                                              T (*create)(std::size_t))
{
    std::vector<std::unique_ptr<T>> buffers;
    buffers.reserve(allocator.FrameSlotCount());
    for (std::uint32_t slot = 0; slot < allocator.FrameSlotCount(); ++slot)
    {
        T buffer = create(size);
        if (!buffer)
            throw std::runtime_error("failed to create an in-flight buffer");
        buffers.push_back(std::make_unique<T>(std::move(buffer)));
    }
    return buffers;
}

void ValidateSlotRange(const InFlightResourceAllocator& allocator, std::uint32_t frameSlot)
{
    if (frameSlot >= allocator.FrameSlotCount())
    {
        throw std::out_of_range("in-flight frame slot is outside the allocator range");
    }
}

void ValidateCurrentSlot(const InFlightResourceAllocator& allocator, std::uint32_t frameSlot)
{
    ValidateSlotRange(allocator, frameSlot);
    if (frameSlot != allocator.CurrentFrameSlot())
    {
        throw std::logic_error("in-flight resource access does not match the Window current frame slot");
    }
}
} // namespace

InFlightResourceAllocator::InFlightResourceAllocator(std::uint32_t frameSlotCount) : m_FrameSlotCount(frameSlotCount)
{
    if (frameSlotCount == 0)
        throw std::invalid_argument("an in-flight resource allocator needs at least one frame slot");
}

void InFlightResourceAllocator::SetCurrentFrame(std::uint32_t frameSlot)
{
    ValidateSlotRange(*this, frameSlot);
    m_CurrentFrameSlot = frameSlot;
}

InFlightUniformBuffer::InFlightUniformBuffer(InFlightResourceAllocator& allocator, std::size_t size) :
    m_Allocator(&allocator),
    m_Buffers(CreateBuffers<rhi::UniformBuffer>(allocator, size, &rhi::UniformBuffer::Create))
{
}

rhi::UniformBuffer& InFlightUniformBuffer::GetBuffer(std::uint32_t frameSlot)
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

const rhi::UniformBuffer& InFlightUniformBuffer::GetBuffer(std::uint32_t frameSlot) const
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

rhi::UniformBuffer& InFlightUniformBuffer::GetBufferForImport(std::uint32_t frameSlot)
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

const rhi::UniformBuffer& InFlightUniformBuffer::GetBufferForImport(std::uint32_t frameSlot) const
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

void InFlightUniformBuffer::SetData(std::uint32_t frameSlot, std::size_t offset, std::span<const std::uint8_t> data)
{
    GetBuffer(frameSlot).SetData(offset, data);
}

InFlightVertexBuffer::InFlightVertexBuffer(InFlightResourceAllocator& allocator, std::size_t size) :
    m_Allocator(&allocator),
    m_Buffers(CreateBuffers<rhi::VertexBuffer>(allocator, size, &rhi::VertexBuffer::Create))
{
}

rhi::VertexBuffer& InFlightVertexBuffer::GetBuffer(std::uint32_t frameSlot)
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

const rhi::VertexBuffer& InFlightVertexBuffer::GetBuffer(std::uint32_t frameSlot) const
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

rhi::VertexBuffer& InFlightVertexBuffer::GetBufferForImport(std::uint32_t frameSlot)
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

const rhi::VertexBuffer& InFlightVertexBuffer::GetBufferForImport(std::uint32_t frameSlot) const
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

void InFlightVertexBuffer::SetData(std::uint32_t frameSlot, std::size_t offset, std::span<const std::uint8_t> data)
{
    GetBuffer(frameSlot).GetVk().SetData(offset, data);
}

InFlightStorageBuffer::InFlightStorageBuffer(InFlightResourceAllocator& allocator, std::size_t size) :
    m_Allocator(&allocator),
    m_Buffers(CreateBuffers<rhi::RWStructuredBuffer>(allocator, size, &rhi::RWStructuredBuffer::Create))
{
}

rhi::RWStructuredBuffer& InFlightStorageBuffer::GetBuffer(std::uint32_t frameSlot)
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

const rhi::RWStructuredBuffer& InFlightStorageBuffer::GetBuffer(std::uint32_t frameSlot) const
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

rhi::RWStructuredBuffer& InFlightStorageBuffer::GetBufferForImport(std::uint32_t frameSlot)
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

const rhi::RWStructuredBuffer& InFlightStorageBuffer::GetBufferForImport(std::uint32_t frameSlot) const
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Buffers[frameSlot];
}

InFlightDescriptorSet::InFlightDescriptorSet(InFlightResourceAllocator& allocator,
                                             const vk::DescriptorSetLayout& layout,
                                             std::uint32_t uniformBufferCount,
                                             std::uint32_t samplerCount,
                                             std::uint32_t storageBufferCount) :
    m_Allocator(&allocator)
{
    m_Sets.reserve(allocator.FrameSlotCount());
    for (std::uint32_t slot = 0; slot < allocator.FrameSlotCount(); ++slot)
    {
        if (uniformBufferCount == 0 && samplerCount == 0 && storageBufferCount == 0)
            throw std::invalid_argument("an in-flight descriptor set needs at least one binding");
        auto poolBuilder = vk::DescriptorPool::Builder().MaxSets(1);
        if (uniformBufferCount)
            poolBuilder.PushUBO(uniformBufferCount);
        if (samplerCount)
            poolBuilder.PushSampler(samplerCount);
        if (storageBufferCount)
            poolBuilder.PushSSBO(storageBufferCount);
        auto pool = poolBuilder.Build();
        if (!pool)
            throw std::runtime_error("failed to create an in-flight descriptor pool");
        auto descriptorSet = vk::DescriptorSet::Create(layout, *pool);
        if (!descriptorSet)
            throw std::runtime_error("failed to create an in-flight descriptor set");
        Slot resource;
        resource.pool = std::move(*pool);
        resource.set = std::move(*descriptorSet);
        m_Sets.push_back(std::move(resource));
    }
}

vk::DescriptorSet& InFlightDescriptorSet::GetSet(std::uint32_t frameSlot)
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Sets[frameSlot].set;
}

const vk::DescriptorSet& InFlightDescriptorSet::GetSet(std::uint32_t frameSlot) const
{
    ValidateCurrentSlot(*m_Allocator, frameSlot);
    return *m_Sets[frameSlot].set;
}

vk::DescriptorSet& InFlightDescriptorSet::GetSetForSetup(std::uint32_t frameSlot)
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Sets[frameSlot].set;
}

const vk::DescriptorSet& InFlightDescriptorSet::GetSetForSetup(std::uint32_t frameSlot) const
{
    ValidateSlotRange(*m_Allocator, frameSlot);
    return *m_Sets[frameSlot].set;
}
} // namespace Aether
