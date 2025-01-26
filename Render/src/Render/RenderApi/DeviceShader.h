#pragma once
#include "../Vulkan/ShaderModule.h"
#include <cassert>
#include <variant>
#include "../Shader/ShaderSource.h"
#include "../Shader/Compiler.h"

namespace Aether
{
struct VulkanShader
{
    vk::ShaderModule vertex;
    vk::ShaderModule fragment;
};
using DeviceShader=std::variant<std::monostate,VulkanShader>;
inline std::expected<DeviceShader,std::string> CreateDeviceShader(const ShaderSource& vert,const ShaderSource frag)
{
    auto vertex=Shader::Compiler::Compile(vert);
    if(!vertex)
    {
        return std::unexpected<std::string>(vertex.error());
    }

    auto fragment=Shader::Compiler::Compile(frag);
    if(!fragment)
    {
        return std::unexpected<std::string>(fragment.error());
    }
    auto vkVertex=vk::ShaderModule::CreateFromBinaryCode(*vertex);
    if(!vkVertex)
    {
        return std::unexpected<std::string>("Failed to create vertex shader module");
    }
    auto vkFragment=vk::ShaderModule::CreateFromBinaryCode(*fragment);
    if(!vkFragment)
    {
        return std::unexpected<std::string>("Failed to create fragment shader module");
    }
    return VulkanShader{std::move(vkVertex.value()),std::move(vkFragment.value())};
}
}