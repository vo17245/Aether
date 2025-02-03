#pragma once
#include "../Config.h"
#include "../Vulkan/Buffer.h"
#include "Render/Config.h"
#include <variant>
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
    template<typename T>
    T& Get()
    {
        return std::get<T>(m_Buffer);
    }
    template<typename T>
    const T& Get() const
    {
        return std::get<T>(m_Buffer);
    }
    void SetData(const uint8_t* data,size_t size)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan:
        {
            auto& buffer = Get<vk::Buffer>();
            buffer.SetData(data, size);
        }
        break;
        default:
        {
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
private:
    std::variant<std::monostate, vk::Buffer> m_Buffer;
};
} // namespace Aether