#pragma once
#include "../Vulkan/Pipeline.h"
#include "Render/Vulkan/Pipeline.h"
#include <variant>
namespace Aether
{
class DevicePipelineLayout
{
public:
    DevicePipelineLayout() = default;
    DevicePipelineLayout(vk::PipelineLayout&& layout) :
        m_Layout(std::move(layout))
    {
    }
    bool Empty() const
    {
        return m_Layout.index() == 0;
    }
    vk::PipelineLayout& GetVk()
    {
        return std::get<vk::PipelineLayout>(m_Layout);
    }
    const vk::PipelineLayout& GetVk() const
    {
        return std::get<vk::PipelineLayout>(m_Layout);
    }
    operator bool() const
    {
        return !Empty();
    }

private:
    std::variant<std::monostate, vk::PipelineLayout> m_Layout;
};
class DevicePipeline
{
public:
    DevicePipeline() = default;
    DevicePipeline(vk::GraphicsPipeline&& pipeline) :
        m_Pipeline(std::move(pipeline))
    {
    }
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