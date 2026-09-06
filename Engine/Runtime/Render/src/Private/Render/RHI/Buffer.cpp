#include <Render/RHI/Buffer.h>
namespace Aether::rhi
{
StagingBuffer StagingBuffer::Create(size_t size)
{
    StagingBuffer stagingBuffer;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto buffer = vk::Buffer::CreateForStaging(size);
        if (!buffer)
        {
            assert(false && "Failed to create staging buffer");
            return StagingBuffer();
        }
        stagingBuffer.m_Buffer = std::move(buffer.value());
        return stagingBuffer;
    }
    break;
    default:
        assert(false && "unsupported api");
        return StagingBuffer();
    }
}
IndexBuffer IndexBuffer::Create(size_t size)
{
    IndexBuffer result;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto buffer = vk::Buffer::CreateForIndex(size);
        if (!buffer)
        {
            assert(false && "Failed to create index buffer");
            return IndexBuffer();
        }
        result.m_Buffer = std::move(buffer.value());
        return result;
    }
    break;
    default:
        assert(false && "unsupported api");
        return IndexBuffer();
    }
}
VertexBuffer VertexBuffer::Create(size_t size)
{
    VertexBuffer result;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto buffer = vk::Buffer::CreateForVertex(size);
        if (!buffer)
        {
            assert(false && "Failed to create vertex buffer");
            return VertexBuffer();
        }
        result.m_Buffer = std::move(buffer.value());
        return result;
    }
    break;
    default:
        assert(false && "unsupported api");
        return VertexBuffer();
    }
}
UniformBuffer UniformBuffer::Create(size_t size)
{
    UniformBuffer stagingBuffer;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto buffer = vk::Buffer::CreateForStaging(size);
        if (!buffer)
        {
            assert(false && "Failed to create staging buffer");
            return UniformBuffer();
        }
        stagingBuffer.m_Buffer = std::move(buffer.value());
        return stagingBuffer;
    }
    break;
    default:
        assert(false && "unsupported api");
        return UniformBuffer();
    }
}
RWStructuredBuffer RWStructuredBuffer::Create(size_t size)
{
    RWStructuredBuffer stagingBuffer;
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto buffer = vk::Buffer::CreateForStaging(size);
        if (!buffer)
        {
            assert(false && "Failed to create staging buffer");
            return RWStructuredBuffer();
        }
        stagingBuffer.m_Buffer = std::move(buffer.value());
        return stagingBuffer;
    }
    break;
    default:
        assert(false && "unsupported api");
        return RWStructuredBuffer();
    }
}
void StagingBuffer::SetData(size_t offset, std::span<const uint8_t> data)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan: {
        auto& vkBuffer = std::get<vk::Buffer>(m_Buffer);
        vkBuffer.SetData(offset, data);
    }
    break;
    default:
        assert(false && "unsupported api");
        break;
    }
}
} // namespace Aether::rhi