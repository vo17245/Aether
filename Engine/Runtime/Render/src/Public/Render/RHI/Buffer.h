#pragma once
#include "../Config.h"
#include "Backend/Vulkan/Buffer.h"
#include "Render/Config.h"
#include <variant>
#include <Render/Id.h>
namespace Aether::rhi
{
class CommandList;
class StagingBuffer
{
    friend class CommandList;
public:
    StagingBuffer() = default;
    StagingBuffer(const StagingBuffer&) = delete;
    StagingBuffer(StagingBuffer&& other) noexcept
    {
        m_Buffer = std::move(other.m_Buffer);
    }
    StagingBuffer& operator=(const StagingBuffer&) = delete;
    StagingBuffer& operator=(StagingBuffer&& other)
    {
        if (this != &other)
        {
            m_Buffer = std::move(other.m_Buffer);
        }
        return *this;
    }
    ~StagingBuffer()
    {
    }

public:
    static StagingBuffer Create(size_t size);
    bool Empty() const
    {
        return m_Buffer.index() == 0;
    }
    const size_t GetSize() const
    {
        if (std::holds_alternative<vk::Buffer>(m_Buffer))
        {
            auto& vkBuffer = std::get<vk::Buffer>(m_Buffer);
            return vkBuffer.GetSize();
        }
        else
        {
            assert(false && "Buffer is empty");
            return 0;
        }
    }
    operator bool() const
    {
        return !Empty();
    }
    /**
     * @param offset Offset in staging buffer to set data
     * @param data Data to set
    */
    void SetData(size_t offset,std::span<const uint8_t> data);

private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};

class VertexBuffer
{
    friend class CommandList;
public:
    VertexBuffer() = default;
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept
    {
        m_Buffer = std::move(other.m_Buffer);
    }
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer& operator=(VertexBuffer&& other)
    {
        if (this != &other)
        {
            m_Buffer = std::move(other.m_Buffer);
        }
        return *this;
    }
    ~VertexBuffer()
    {
    }

public:
static VertexBuffer Create(size_t size);
    bool Empty() const
    {
        return m_Buffer.index() == 0;
    }
    const size_t GetSize() const
    {
        if (std::holds_alternative<vk::Buffer>(m_Buffer))
        {
            auto& vkBuffer = std::get<vk::Buffer>(m_Buffer);
            return vkBuffer.GetSize();
        }
        else
        {
            assert(false && "Buffer is empty");
            return 0;
        }
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};
class IndexBuffer
{
    friend class CommandList;
public:
    IndexBuffer() = default;
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&& other) noexcept
    {
        m_Buffer = std::move(other.m_Buffer);
    }
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer& operator=(IndexBuffer&& other)
    {
        if (this != &other)
        {
            m_Buffer = std::move(other.m_Buffer);
        }
        return *this;
    }
    ~IndexBuffer()
    {
    }

public:
static IndexBuffer Create(size_t size);
    bool Empty() const
    {
        return m_Buffer.index() == 0;
    }
    const size_t GetSize() const
    {
        if (std::holds_alternative<vk::Buffer>(m_Buffer))
        {
            auto& vkBuffer = std::get<vk::Buffer>(m_Buffer);
            return vkBuffer.GetSize();
        }
        else
        {
            assert(false && "Buffer is empty");
            return 0;
        }
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};
class UniformBuffer
{
    friend class CommandList;
public:
    UniformBuffer() = default;
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&& other) noexcept
    {
        m_Buffer = std::move(other.m_Buffer);
    }
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer& operator=(UniformBuffer&& other)
    {
        if (this != &other)
        {
            m_Buffer = std::move(other.m_Buffer);
        }
        return *this;
    }
    ~UniformBuffer()
    {
    }

public:
static UniformBuffer Create(size_t size);
    bool Empty() const
    {
        return m_Buffer.index() == 0;
    }
    const size_t GetSize() const
    {
        if (std::holds_alternative<vk::Buffer>(m_Buffer))
        {
            auto& vkBuffer = std::get<vk::Buffer>(m_Buffer);
            return vkBuffer.GetSize();
        }
        else
        {
            assert(false && "Buffer is empty");
            return 0;
        }
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};
class RWStructuredBuffer
{
    friend class CommandList;
public:
    RWStructuredBuffer() = default;
    RWStructuredBuffer(const RWStructuredBuffer&) = delete;
    RWStructuredBuffer(RWStructuredBuffer&& other) noexcept
    {
        m_Buffer = std::move(other.m_Buffer);
    }
    RWStructuredBuffer& operator=(const RWStructuredBuffer&) = delete;
    RWStructuredBuffer& operator=(RWStructuredBuffer&& other)
    {
        if (this != &other)
        {
            m_Buffer = std::move(other.m_Buffer);
        }
        return *this;
    }
    ~RWStructuredBuffer()
    {
    }

public:
static RWStructuredBuffer Create(size_t size);
    bool Empty() const
    {
        return m_Buffer.index() == 0;
    }
    const size_t GetSize() const
    {
        if (std::holds_alternative<vk::Buffer>(m_Buffer))
        {
            auto& vkBuffer = std::get<vk::Buffer>(m_Buffer);
            return vkBuffer.GetSize();
        }
        else
        {
            assert(false && "Buffer is empty");
            return 0;
        }
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};
using AllBufferTypes=TypeArray<VertexBuffer,IndexBuffer,UniformBuffer,RWStructuredBuffer>;
template<typename T>
concept IsBufferType=IsArrayContainType<T,AllBufferTypes>::value;
} // namespace Aether::rhi