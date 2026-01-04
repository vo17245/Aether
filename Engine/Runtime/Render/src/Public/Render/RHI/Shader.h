#pragma once
#include "Backend/Vulkan/ShaderModule.h"
#include <cassert>
#include <variant>
#include "../Shader/ShaderSource.h"
#include "../Shader/Compiler.h"
#include "Render/Config.h"
namespace Aether::rhi
{
class VertexShader
{
public:
    static std::expected<VertexShader, std::string> Create(const ShaderSource& source);
    VertexShader() = default;
    vk::ShaderModule& GetVk()
    {
        return std::get<vk::ShaderModule>(m_Shader);
    }
    const vk::ShaderModule& GetVk() const
    {
        return std::get<vk::ShaderModule>(m_Shader);
    }
    bool Empty() const
    {
        return m_Shader.index() == 0;
    }

private:
    static std::expected<VertexShader, std::string> CreateVk(const ShaderSource& source);
    std::variant<std::monostate, vk::ShaderModule> m_Shader;
};
class PixelShader
{
public:
    static std::expected<PixelShader, std::string> Create(const ShaderSource& source);
    PixelShader() = default;
    vk::ShaderModule& GetVk()
    {
        return std::get<vk::ShaderModule>(m_Shader);
    }
    const vk::ShaderModule& GetVk() const
    {
        return std::get<vk::ShaderModule>(m_Shader);
    }
    bool Empty() const
    {
        return m_Shader.index() == 0;
    }

private:
    static std::expected<PixelShader, std::string> CreateVk(const ShaderSource& source);
    std::variant<std::monostate, vk::ShaderModule> m_Shader;
};

} // namespace Aether::rhi