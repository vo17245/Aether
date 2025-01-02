#pragma once

#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <memory>
#include <optional>
namespace Aether {
namespace vk {

class Fence
{
public:
    static std::optional<Fence> Create(bool signaled = false);
    static std::unique_ptr<Fence> CreateScope(bool signaled = false);
    static Fence* CreatePtr(bool signaled = false);
    ~Fence();
    VkFence GetHandle() const;
    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;
    Fence(Fence&& other) noexcept;
    Fence& operator=(Fence&& other) noexcept;
    void Reset();
    //@return
    //  Result::None ok
    //  Result::Timeout timeout
    VkResult Wait(uint64_t timeout = UINT64_MAX);

private:
    Fence(VkFence fence) :
        m_Fence(fence)
    {
    }
    VkFence m_Fence;
};
}
} // namespace Aether::vk