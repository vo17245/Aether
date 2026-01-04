#include <Render/RHI/Shader.h>

namespace Aether::rhi
{
std::expected<VertexShader, std::string> VertexShader::CreateVk(const ShaderSource& source)
{
    VertexShader res;
    auto vertex = Shader::Compiler::Compile(source);
    if (!vertex)
    {
        return std::unexpected<std::string>(vertex.error());
    }
    auto vkVertex = vk::ShaderModule::CreateFromBinaryCode(*vertex);
    if (!vkVertex)
    {
        return std::unexpected<std::string>("Failed to create vertex shader module");
    }
    res.m_Shader = std::move(vkVertex.value());
    return res;
}
std::expected<VertexShader, std::string> VertexShader::Create(const ShaderSource& source)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan:
        return CreateVk(source);
    default:
        return std::unexpected<std::string>("Not implemented render backend");
    }
}
std::expected<PixelShader, std::string> PixelShader::CreateVk(const ShaderSource& source)
{
    PixelShader res;
    auto vertex = Shader::Compiler::Compile(source);
    if (!vertex)
    {
        return std::unexpected<std::string>(vertex.error());
    }
    auto vkVertex = vk::ShaderModule::CreateFromBinaryCode(*vertex);
    if (!vkVertex)
    {
        return std::unexpected<std::string>("Failed to create vertex shader module");
    }
    res.m_Shader = std::move(vkVertex.value());
    return res;
}
std::expected<PixelShader, std::string> PixelShader::Create(const ShaderSource& source)
{
    switch (Render::Config::RenderApi)
    {
    case Render::Api::Vulkan:
        return CreateVk(source);
    default:
        return std::unexpected<std::string>("Not implemented render backend");
    }
}
} // namespace Aether::rhi