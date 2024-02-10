#pragma once
#include "Aether/Render/ShaderSource.h"
#include "ShaderSource.h"
#include "Shader.h"
namespace Aether
{
    class ShaderCache
    {
    public:
        Ref<Shader> GetShader(BuiltinShaderSignature signature,BuiltinShaderSpecSet specSet);
        static ShaderCache& GetInstance();
    private:
        
    };
}