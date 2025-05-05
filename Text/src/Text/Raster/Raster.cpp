#include "Raster.h"
#include "QuadArrayMesh.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Quad.h"
namespace Aether::Text
{
void Raster::Render(RenderPassParam& param,RenderPassResource& resource)
{
    auto& commandBuffer=param.commandBuffer;
    auto& renderPass=param.renderPass;
    auto& frameBuffer=param.frameBuffer;
    
}
bool Raster::Init(DeviceRenderPassView renderPass, bool enableBlend,DeviceDescriptorPool& descriptorPool)
{
    if(!CreateDescriptorSet(descriptorPool))
    {
        assert(false && "create descriptor set failed");
        return false;
    }
    if (!CreateShader())
    {
        assert(false && "create shader failed");
        return false;
    }
    if (!CreatePipeline(renderPass, enableBlend))
    {
        assert(false && "create pipeline failed");
        return false;
    }
    return true;
}
bool Raster::CreateShader()
{
    static const char* frag = R"(
#version 450
layout(location=0)in uint v_Color;
layout(location=0)out vec4 FragColor;
void main()
{
    FragColor=vec4(1.0,1.0,1.0,1.0);
}
)";
    static const char* vert = R"(
#version 450
layout(location=0)in vec3 a_Position;
layout(location=1)in uint a_GlyphIndex;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_MVP;
}ubo;
layout(location=0)out uint v_GlyphIndex;
void main()
{
    vec4 pos=vec4(a_Position,1.0);
    v_GlyphIndex=a_GlyphIndex;
    gl_Position=ubo.u_MVP*pos;
}
)";
    auto shaderEx = DeviceShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vert),
                                         ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, frag));
    if (!shaderEx)
    {
        assert(false && "create shader failed");
        return false;
    }
    m_Shader = std::move(shaderEx.value());
    return true;
}
bool Raster::CreatePipeline(DeviceRenderPassView renderPass, bool enableBlend)
{
    vk::PipelineLayout::Builder layoutBuilder;
    layoutBuilder.AddDescriptorSetLayouts(m_DescriptorSet.GetVk().layouts);
    auto layoutOpt = layoutBuilder.Build();
    if (!layoutOpt)
    {
        assert(false && "failed to create pipeline layout");
        return false;
    }
    auto& layout = layoutOpt.value();
    vk::GraphicsPipeline::Builder builder(renderPass.GetVk(), layout);
    QuadArrayMesh dummyMesh;
    auto vertexBufferLayouts = dummyMesh.GetMesh().CreateVertexBufferLayouts();
    builder.PushVertexBufferLayouts(vertexBufferLayouts)
        .AddVertexStage(m_Shader.GetVk().vertex, "main")
        .AddFragmentStage(m_Shader.GetVk().fragment, "main");
    if (enableBlend)
    {
        builder.EnableBlend();
    }
    auto pipelineOpt = builder.Build();
    if (!pipelineOpt)
    {
        assert(false && "failed to create pipeline");
        return false;
    }
    auto& pipeline = pipelineOpt.value();

    m_Pipeline = std::move(pipeline);
    m_PipelineLayout = std::move(layout);
    return true;
}
bool Raster::CreateCamera(const Vec2f& screenSize)
{
    m_Camera.screenSize = screenSize;
    m_Camera.target=Vec2f(screenSize.x()/2,screenSize.y()/2);
    m_Camera.offset=Vec2f(0,0);
    m_Camera.near=0.0f;
    m_Camera.far=10000.0f;
    m_Camera.zoom=1.0f;
    m_Camera.rotation=0.0f;
    m_Camera.CalculateMatrix();
    return true;
}

bool Raster::CreateDescriptorSet(DeviceDescriptorPool& descriptorPool)
{
    auto set=descriptorPool.CreateSet(1, 0, 0);
    m_DescriptorSet = std::move(set);
    return true;
}
void Raster::UpdateMesh(RenderPassParam& param,RenderPassResource& resource)
{
// create host mesh data
}
} // namespace Aether::Text