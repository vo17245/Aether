
#include "doctest/doctest.h"
#include "Render/Render.h"
using namespace Aether;

static const char* vert=R"(
// 输入顶点结构
struct VSInput
{
    float3 position : POSITION;  // 顶点位置
    float3 normal   : NORMAL;    // 法线，可选
};

// 输出顶点结构
struct VSOutput
{
    float4 position : SV_POSITION; // 裁剪空间位置
};

// 顶点着色器入口
VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = float4(input.position, 1.0f);
    return output;
}

)";
static const char* frag=R"(
// 输入片段结构
struct PSInput
{
    float4 position : SV_POSITION; // 裁剪空间位置
};
cbuffer MyConstants : register(b0)
{
    float4 someValue;
};
// 输出颜色
float4 main(PSInput input) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0); // 输出红色
}
)";
TEST_CASE("Test Preprocessor")
{
    auto spirv=Shader::Compiler::HLSL2SPIRV(&vert, 1, ShaderStageType::Vertex);
    REQUIRE(spirv.has_value());
    auto map=ReflectShaderParameters(spirv.value());
    
    
}