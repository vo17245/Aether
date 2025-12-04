#pragma once
#include "../Vulkan/GraphicsCommandPool.h"
#include <variant>
#include "../Config.h"
#include <cassert>
#include "DeviceCommandBuffer.h"
namespace Aether
{
class DeviceCommandBufferPool
{
public:
    DeviceCommandBufferPool(DeviceCommandBufferPool&& other) = default;
    DeviceCommandBufferPool& operator=(DeviceCommandBufferPool&& other) = default;
    DeviceCommandBufferPool(const DeviceCommandBufferPool& other) = delete;
    DeviceCommandBufferPool& operator=(const DeviceCommandBufferPool& other) = delete;
    ~DeviceCommandBufferPool() = default;
    DeviceCommandBufferPool() = default;

public:
    vk::GraphicsCommandPool& GetVk()
    {
        return std::get<vk::GraphicsCommandPool>(m_Pool);
    }
    bool Empty() const
    {
        return std::holds_alternative<std::monostate>(m_Pool);
    }
    static DeviceCommandBufferPool Create()
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto pool = vk::GraphicsCommandPool::Create();
            if (!pool)
            {
                assert(false && "failed to create command buffer pool");
                return DeviceCommandBufferPool();
            }
            DeviceCommandBufferPool commandBufferPool;
            commandBufferPool.m_Pool = std::move(pool.value());
            return commandBufferPool;
        }
        default:
            assert(false && "unknown render api");
            return DeviceCommandBufferPool();
        }
    }
    DeviceCommandBuffer AllocateCommandBuffer()
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan: {
            auto commandBuffer = vk::GraphicsCommandBuffer::Create(GetVk());
            if (!commandBuffer)
            {
                assert(false && "failed to allocate command buffer");
                return DeviceCommandBuffer();
            }
            return DeviceCommandBuffer(std::move(commandBuffer.value()));
        }
        default:
            assert(false && "unknown render api");
            return DeviceCommandBuffer();
        }
    }


private:
    std::variant<std::monostate, vk::GraphicsCommandPool> m_Pool;
};
} // namespace Aether