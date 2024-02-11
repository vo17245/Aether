#pragma once
#include "Aether/Render/ShaderSource.h"
#include "ShaderSource.h"
#include "Shader.h"
#include <stdint.h>
#include <unordered_map>
namespace Aether
{
    class ShaderCache
    {
        friend class Application;
    public:
        Ref<Shader> GetShader(BuiltinShaderSignature signature);
        static ShaderCache& GetInstance()
        {
            static ShaderCache cache;
            return cache;
        }
    private:
        ShaderCache() = default;
        //uint64_t BuiltinShaderSignature::value
        std::unordered_map<uint64_t,Ref<Shader> > m_ShaderCache;
        //uint32_t BuiltinShaderType
        std::unordered_map<uint32_t, Ref<ShaderSource>> m_SourceCache;
    };
}