#pragma once
#include "vulkan/vulkan_core.h"
#include <optional>
#include "Core/Base.h"
namespace Aether {
namespace vk {

class Semaphore
{
public:
    static std::optional<Semaphore> Create();
    static Scope<Semaphore> CreateScope();
    ~Semaphore();
    VkSemaphore GetHandle() const;
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore(Semaphore&& other) noexcept;
    Semaphore& operator=(Semaphore&& other) noexcept;
    //@breif
    // 进行一次空的队列提交，信号这个 semaphore，阻塞cpu直到这个动作完成
    // 仅debug用
    void Signal();
    //@breif
    // 进行一次空的队列提交，等待这个 semaphore，阻塞cpu直到这个动作完成
    // 仅debug用
    void Wait();

private:
    Semaphore(VkSemaphore semaphore) :
        m_Semaphore(semaphore)
    {
    }
    VkSemaphore m_Semaphore;
};
}
} // namespace Aether::vk