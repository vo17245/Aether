#pragma once
#include "../Core/Core.h"
#include <filesystem>
#include <optional>
namespace Aether
{
    enum class BuiltinShaderType:uint32_t
    {
        PBR,
        BASIC,
    };
    enum class BuiltinShaderPbrMacro:uint32_t
    {
        USE_NORMAL_MAP=Bit(0),
        USE_ALBEDO_MAP=Bit(1),
    };
    using BuiltinShaderMacro=uint32_t;
    struct BuiltinShaderSignature
    {
        union
        {
            struct
            {
                BuiltinShaderType type;
                BuiltinShaderMacro macro;
            };
            uint64_t value;
        };
        BuiltinShaderSignature(BuiltinShaderType type,BuiltinShaderMacro macro=0)
        :type(type),macro(macro){}
    };
    
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
        static std::optional<Ref<ShaderSource>> LoadBuiltin(BuiltinShaderType type); 
    private:
        ShaderSource() {}
        std::string m_VertexSource;
        std::string m_FragmentSource;
    };
}