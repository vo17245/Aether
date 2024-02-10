#pragma once
#include "../Core/Core.h"
#include <filesystem>
#include <optional>
namespace Aether
{
    enum class BuiltinShaderSignature
    {
        PBR,
        BASIC,
    };
    using BultinShaderSpec=uint64_t;
    using BuiltinShaderSpecSet=uint64_t;
    
    class ShaderSource
    {
    public:
        void AddLineInVertexShader(const std::string& line);
        void AddLineInFragmentShader(const std::string& line);
        void AddVertexShaderMacro(const std::string& macro);
        void AddFragmentShaderMacro(const std::string& macro);
        const std::string& GetVertexSource()const{return m_VertexSource;}
        const std::string& GetFragmentSource()const{return m_FragmentSource;}

    public:
        static std::optional<Ref<ShaderSource>> LoadFromFile(const std::filesystem::path& vsPath,
        const std::filesystem::path& fsPath);
        static std::optional<Ref<ShaderSource>> LoadBuiltin(BuiltinShaderSignature signature); 
    private:
        ShaderSource();
        std::string m_VertexSource;
        std::string m_FragmentSource;
    };
}