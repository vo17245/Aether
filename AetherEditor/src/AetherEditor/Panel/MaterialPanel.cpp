#include "MaterialPanel.h"
#include <Debug/Log.h>
namespace AetherEditor::UI
{

void MaterialPanel::TestReflection()
{
    auto code=Filesystem::ReadFileToString("tmp/Material/frag.hlsl");
    assert(code);
    const auto* data=code->data();
    auto spirv=Shader::Compiler::HLSL2SPIRV(&data,1, ShaderStageType::Fragment);
    if(!spirv)
    {
        Debug::Log::Error("Failed to compile shader: {}",spirv.error());
    }
    assert(spirv.has_value());
    auto map=ReflectShaderParameters(spirv.value());
    assert(map.has_value());
}
}