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
        assert(std::holds_alternative<vk::Sampler>(m_Sampler) && "not vk sampler");
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
    /**
     * linear filter in mag and min and mipmap
     * always repeat
     * always use 0 mipmap level
     * */ 
    static DeviceSampler CreateDefault()
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan:
        {
            vk::Sampler::Builder builder;
            builder.SetDefaultCreateInfo();
            auto sampler = builder.Build();
            if (!sampler)
            {
                return DeviceSampler();
            }
            return DeviceSampler(std::move(sampler.value()));
        }
        default:
            assert(false && "Not implemented");
            return DeviceSampler();
        }
    }
private:
    std::variant<std::monostate, vk::Sampler> m_Sampler;
};
} // namespace Aether