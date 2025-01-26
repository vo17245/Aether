#pragma once
#include <string>
namespace Aether {
enum class ShaderStageType
{
    Vertex,
    Fragment,
    Compute
};
enum class ShaderLanguage
{
    GLSL,
    HLSL
};
class ShaderSource
{
public:
    ShaderSource(ShaderStageType stageType, ShaderLanguage language, const std::string& code) :
        m_StageType(stageType),
        m_Language(language),
        m_Code(code)
    {
    }
    ShaderStageType GetStageType() const
    {
        return m_StageType;
    }
    ShaderLanguage GetLanguage() const
    {
        return m_Language;
    }
    const std::string& GetCode() const
    {
        return m_Code;
    }
private:
    ShaderStageType m_StageType;
    ShaderLanguage m_Language;
    std::string m_Code;
};
} // namespace Aether