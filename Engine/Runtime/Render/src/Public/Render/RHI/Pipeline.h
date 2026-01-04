#pragma once
#include "Backend/Vulkan/Pipeline.h"
#include "Backend/Vulkan/Pipeline.h"
#include <variant>
namespace Aether::rhi
{

class DevicePipeline
{
public:
    DevicePipeline() = default;
    bool Empty() const
    {
        return m_Pipeline.index() == 0;
    }
    vk::GraphicsPipeline& GetVk()
    {
        return std::get<vk::GraphicsPipeline>(m_Pipeline);
    }
    const vk::GraphicsPipeline& GetVk() const
    {
        return std::get<vk::GraphicsPipeline>(m_Pipeline);
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::GraphicsPipeline> m_Pipeline;
};
} // namespace Aether