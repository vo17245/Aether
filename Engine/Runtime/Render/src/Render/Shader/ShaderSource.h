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
    ShaderSource(ShaderStageType stageType, ShaderLanguage language, const std::string& code,const std::string& preamble) :
        m_StageType(stageType),
        m_Language(language),
        m_Code(code),
        m_Preamble(preamble)
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
    const std::string& GetPreamble() const
    {
        return m_Preamble;
    }
    void SetPreamble(const std::string& preamble)
    {
        m_Preamble = preamble;
    }
private:
    ShaderStageType m_StageType;
    ShaderLanguage m_Language;
    std::string m_Code;
    std::string m_Preamble; // optional preamble for shader code
};
} // namespace Aether