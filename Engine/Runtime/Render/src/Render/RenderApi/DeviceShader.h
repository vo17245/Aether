#pragma once
#include "../Vulkan/ShaderModule.h"
#include <cassert>
#include <variant>
#include "../Shader/ShaderSource.h"
#include "../Shader/Compiler.h"
#include "Render/Config.h"
namespace Aether
{
struct VulkanShader
{
    vk::ShaderModule vertex;
    vk::ShaderModule fragment;
};
class DeviceShader
{
public:
    static std::expected<DeviceShader, std::string> Create(const ShaderSource& vert, const ShaderSource& frag)
    {
        switch (Render::Config::RenderApi)
        {
        case Render::Api::Vulkan:
            return CreateVk(vert, frag);
        default:
            return std::unexpected<std::string>("Not implemented render backend");
        }
    }
    DeviceShader() = default;
    
    template<typename T>
    T& Get()
    {
        return std::get<T>(m_Shader);
    }
    template<typename T>
    const T& Get() const
    {
        return std::get<T>(m_Shader);
    }
    VulkanShader& GetVk()
    {
        return Get<VulkanShader>();
    }
    const VulkanShader& GetVk() const
    {
        return Get<VulkanShader>();
    }
    bool Empty() const
    {
        return m_Shader.index() == 0;
    }

private:
    static std::expected<DeviceShader, std::string> CreateVk(const ShaderSource& vert, const ShaderSource& frag)
    {
        DeviceShader res;
        auto vertex = Shader::Compiler::Compile(vert);
        if (!vertex)
        {
            return std::unexpected<std::string>(vertex.error());
        }

        auto fragment = Shader::Compiler::Compile(frag);
        if (!fragment)
        {
            return std::unexpected<std::string>(fragment.error());
        }
        auto vkVertex = vk::ShaderModule::CreateFromBinaryCode(*vertex);
        if (!vkVertex)
        {
            return std::unexpected<std::string>("Failed to create vertex shader module");
        }
        auto vkFragment = vk::ShaderModule::CreateFromBinaryCode(*fragment);
        if (!vkFragment)
        {
            return std::unexpected<std::string>("Failed to create fragment shader module");
        }
        res.m_Shader = VulkanShader{std::move(vkVertex.value()), std::move(vkFragment.value())};
        return res;
    }
    std::variant<std::monostate, VulkanShader> m_Shader;
};

} // namespace Aether