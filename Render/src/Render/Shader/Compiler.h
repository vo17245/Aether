#pragma once
#include <vector>
#include <optional>
#include <expected>
#include <string>
#include <span>
namespace Aether {
namespace Shader
{
    enum class ShaderType
    {
        Vertex,
        Fragment,
        Compute
    };
    class Compiler
    {
    public:
        static std::expected<std::vector<uint32_t>,std::string> GLSL2SPIRV(const char** codes,size_t codeCnt,ShaderType shaderType);
    };
}
}