#pragma once
#include <Render/Vulkan/Semaphore.h>
#include <variant>
#include <Render/Config.h>
#include <Core/Core.h>
namespace Aether
{
    class DeviceSemaphore
    {
    public:
        DeviceSemaphore() = default;
        DeviceSemaphore(const DeviceSemaphore&) = delete;
        DeviceSemaphore(DeviceSemaphore&& other) =default;
        DeviceSemaphore& operator=(const DeviceSemaphore&) = delete;
        DeviceSemaphore& operator=(DeviceSemaphore&& other)=default;
        ~DeviceSemaphore() = default;
    public:
        bool Empty() const
        {
            return m_Semaphore.index() == 0;
        }
        static DeviceSemaphore Create()
        {
            DeviceSemaphore res;
            switch (Render::Config::RenderApi)
            {
            case Render::Api::Vulkan: {
                auto semaphore = vk::Semaphore::Create();
                if (!semaphore)
                {
                    return res;
                }
                res.m_Semaphore = std::move(semaphore.value());
            }
            break;
            default:
                assert(false && "Not implemented");
                return res;
            }
            return res;
        }
    private:
        std::variant<std::monostate, vk::Semaphore> m_Semaphore;
    };
}