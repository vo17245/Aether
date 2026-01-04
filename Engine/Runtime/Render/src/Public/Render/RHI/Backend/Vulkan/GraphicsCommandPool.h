#pragma once
#include "Core/Base.h"
#include <optional>
#include "vulkan/vulkan_core.h"
namespace Aether {
namespace vk {
class GraphicsCommandPool
{
public:
    static std::optional<GraphicsCommandPool> Create();
    static Scope<GraphicsCommandPool> CreateScope();
    GraphicsCommandPool(GraphicsCommandPool&& other) noexcept;
    GraphicsCommandPool& operator=(GraphicsCommandPool&& other) noexcept;
    ~GraphicsCommandPool();
    VkCommandPool GetHandle() const;

private:
    GraphicsCommandPool(VkCommandPool handle) :
        m_Handle(handle)
    {
    }
    VkCommandPool m_Handle;
};
}
} // namespace Aether::vk