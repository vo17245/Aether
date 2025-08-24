#pragma once
#include "../Config.h"
#include "../Vulkan/Buffer.h"
#include "Render/Config.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include <variant>
#include "Render/Vulkan/Transfer.h"
namespace Aether
{
class DeviceBuffer
{
public:
    DeviceBuffer() = default;
    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer(DeviceBuffer&&) = default;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(DeviceBuffer&&) = default;
    bool Empty() const
    {
        return m_Buffer.index() == 0;
    }
    static DeviceBuffer CreateForUniform(size_t size)
    {
        DeviceBuffer res;
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto buffer = vk::Buffer::CreateForUBO(size);
            if (!buffer)
            {
                return res;
            }
            res.m_Buffer = std::move(buffer.value());
            return res;
        }
        default: {
            assert(false && "Not implemented");
            return res;
        }
        }
    }
    static DeviceBuffer CreateForStaging(size_t size)
    {
        DeviceBuffer res;
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto buffer = vk::Buffer::CreateForStaging(size);
            if (!buffer)
            {
                return res;
            }
            res.m_Buffer = std::move(buffer.value());
            return res;
        }
        break;
        default: {
            assert(false && "Not implemented");
            return res;
        }
        }
    }
    static DeviceBuffer CreateForSSBO(size_t size)
    {
        DeviceBuffer res;
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto buffer = vk::Buffer::CreateForSSBO(size);
            if (!buffer)
            {
                return res;
            }
            res.m_Buffer = std::move(buffer.value());
            return res;
        }
        break;
        default: {
            assert(false && "Not implemented");
            return res;
        }
        }
    }
    static DeviceBuffer CreateForVBO(size_t size)
    {
        DeviceBuffer res;
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto buffer = vk::Buffer::CreateForVertex(size);
            if (!buffer)
            {
                return res;
            }
            res.m_Buffer = std::move(buffer.value());
            return res;
        }
        break;
        default: {
            assert(false && "Not implemented");
            return res;
        }
        }
    }
    template <typename T>
    T& Get()
    {
        return std::get<T>(m_Buffer);
    }
    template <typename T>
    const T& Get() const
    {
        return std::get<T>(m_Buffer);
    }
    void SetData(const uint8_t* data, size_t size)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto& buffer = Get<vk::Buffer>();
            buffer.SetData(data, size);
        }
        break;
        default: {
            assert(false && "Not implemented");
        }
        }
    }
    void SetData(std::span<const uint8_t> data)
    {
        SetData(data.data(), data.size());
    }
    vk::Buffer& GetVk()
    {
        return Get<vk::Buffer>();
    }
    const vk::Buffer& GetVk() const
    {
        return Get<vk::Buffer>();
    }
    const size_t GetSize() const
    {
        if (std::holds_alternative<vk::Buffer>(m_Buffer))
        {
            auto& vkBuffer = GetVk();
            return vkBuffer.GetSize();
        }
        else
        {
            assert(false && "not implement");
            return 0;
        }
    }
    template <typename T>
    bool IsType() const
    {
        return std::holds_alternative<T>(m_Buffer);
    }
    /**
     * @brief copy to this buffer
     */
    static bool SyncCopy(DeviceBuffer& _dst, DeviceBuffer& _src, size_t size, size_t dstOffset, size_t srcOffset)
    {
        if (_dst.IsType<vk::Buffer>() && _src.IsType<vk::Buffer>())
        {
            auto& dst = _dst.GetVk();
            auto& src = _src.GetVk();
            return vk::Buffer::SyncCopy(src, dst, size, srcOffset, dstOffset);
        }
        else
        {
            assert(false && "not implement");
            return false;
        }
    }
    operator bool() const
    {
        return !Empty();
    }
    

private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};
} // namespace Aether