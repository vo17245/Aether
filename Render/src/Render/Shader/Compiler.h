#pragma once
#include <vector>
#include <optional>
#include <expected>
#include <string>
#include <span>
#include <cstdint>
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
        static void Init();
        static void Shutdown();
        static std::expected<std::vector<uint32_t>,std::string> GLSL2SPIRV(const char** codes,size_t codeCnt,ShaderType shaderType);
        static std::expected<std::vector<uint32_t>,std::string> HLSL2SPIRV(const char** codes,size_t codeCnt,ShaderType shaderType);
    };
}
}