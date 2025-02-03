#pragma once
#include <variant>
#include "../Vulkan/Sampler.h"
#include "Render/Vulkan/Sampler.h"
#include "../Config.h"
namespace Aether
{
class DeviceSampler
{
public:
    DeviceSampler() = default;
    DeviceSampler(vk::Sampler&& sampler) :
        m_Sampler(std::move(sampler))
    {
    }
    bool Empty() const
    {
        return m_Sampler.index() == 0;
    }
    vk::Sampler& GetVk()
    {
        return std::get<vk::Sampler>(m_Sampler);
    }
    const vk::Sampler& GetVk() const
    {
        return std::get<vk::Sampler>(m_Sampler);
    }
    operator bool() const
    {
        return !Empty();
    }
private:
    std::variant<std::monostate, vk::Sampler> m_Sampler;
};
} // namespace Aether