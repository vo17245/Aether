#include "MaterialPanel.h"
void MaterialPanel::TestReflection()
{
    auto code=Filesystem::ReadFileToString("tmp/test_frag.hlsl");
    assert(code);
    const auto* data=code->data();
    auto spirv=Shader::Compiler::HLSL2SPIRV(&data,1, ShaderStageType::Fragment);
    auto map=ReflectShaderParameters(spirv.value());
}