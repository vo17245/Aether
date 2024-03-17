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
        SKYBOX,
        GAUSSIAN_BLUR,
        SCREEN_QUAD,
    };
    enum class BuiltinShaderPbrMacro:uint32_t
    {
        USE_AO_TEX=Bit(0),
        USE_ROUGHNESS_TEX=Bit(1),
        USE_ALBEDO_TEX=Bit(2),
        USE_METALLIC_TEX=Bit(3),
        IBL=Bit(4),
        TONEMAPPING=Bit(5),
        GAMMA=Bit(6),
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
        ShaderSource(const ShaderSource&) = default;
        ShaderSource(ShaderSource&&) = default;
        void AddLineInVertexShader(const std::string& line);
        void AddLineInFragmentShader(const std::string& line);
        void AddVertexShaderMacro(const std::string& macro);
        void AddFragmentShaderMacro(const std::string& macro);
        const std::string& GetVertexSource()const{return m_VertexSource;}
        const std::string& GetFragmentSource()const{return m_FragmentSource;}
        void UseMacro(BuiltinShaderSignature signature);

    public:
        static std::optional<Ref<ShaderSource>> LoadFromFile(const std::filesystem::path& vsPath,
        const std::filesystem::path& fsPath);
        static std::optional<Ref<ShaderSource>> LoadBuiltin(BuiltinShaderType type); 
        ShaderSource(const std::string& vs,const std::string& fs)
            :m_VertexSource(vs),m_FragmentSource(fs){}
    private:
        ShaderSource() {}
        std::string m_VertexSource;
        std::string m_FragmentSource;
    };
}