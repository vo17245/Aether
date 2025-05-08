#include "Raster.h"
#include "QuadArrayMesh.h"
#include "Render/RenderApi/DeviceBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceMesh.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Quad.h"
#include "PackGlyph.h"
#include "Render/Utils.h"
namespace Aether::Text
{
bool Raster::Render(RenderPassParam& param, RenderPassResource& resource)
{
    bool res;
    // update mesh  and uniform
    res = UpdateMesh(param, resource);
    if (!res)
    {
        return false;
    }
    res = UpdateUniformBuffer(param, resource);
    if (!res)
    {
        return false;
    }
    res = CreateDescriptorSet(param.descriptorPool);
    if (!res)
    {
        return false;
    }
    res = UpdateDescriptorSet(resource);
    if (!res)
    {
        return false;
    }
    res = RecordCommand(param, resource);
    if (!res)
    {
        return false;
    }

    return true;
}
bool Raster::Init(DeviceRenderPassView renderPass, bool enableBlend, DeviceDescriptorPool& descriptorPool,
const Vec2f& screenSize)
{
    if (!CreateDescriptorSet(descriptorPool))
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
    CreateCamera(screenSize);
    m_StaggingBuffer=DeviceBuffer::CreateForStaging(sizeof(HostUniformBuffer));
    return true;
}
bool Raster::CreateShader()
{
    static const char* frag = R"(
#version 450
layout(location=0)flat in uint v_Color;
layout(location=0)out vec4 FragColor;

void main()
{
    
    FragColor=vec4(0.0,1.0,0.0,1.0);
}
)";
    static const char* vert = R"(
#version 450
layout(location=0)in vec3 a_Position;
layout(location=1)in uint a_GlyphIndex;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_MVP;
    vec4 packed;
}ubo;
vec2 positions[6] = vec2[](
    vec2(-1.0f, 1.0),
    vec2(1.0f, -1.0),
    vec2(1.0f, 1.0),
    vec2(-1.0f, 1.0),
    vec2(-1.0f, -1.0f),
    vec2(1.0f, -1.0f)
);
layout(location=0)flat out uint v_GlyphIndex;
void main()
{
    vec4 pos=vec4(a_Position,1.0);
    v_GlyphIndex=a_GlyphIndex;
    gl_Position=ubo.u_MVP*pos;
    //vec2 debug_pos=positions[gl_VertexIndex%6];
    //gl_Position=vec4(debug_pos.x,debug_pos.y,0.0,1.0);
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
    m_Camera.target = Vec2f(screenSize.x() / 2, screenSize.y() / 2);
    m_Camera.offset = Vec2f(0, 0);
    m_Camera.near = 0.0f;
    m_Camera.far = 10000.0f;
    m_Camera.zoom = 1.0f;
    m_Camera.rotation = 0.0f;
    m_Camera.CalculateMatrix();
    return true;
}

bool Raster::CreateDescriptorSet(DeviceDescriptorPool& descriptorPool)
{
    auto set = descriptorPool.CreateSet(1, 0, 0);
    m_DescriptorSet = std::move(set);
    return true;
}
static Quad CreateQuad(const Font::Glyph& glyph, const Vec2f& pos, uint32_t glyphIndex)
{
    Quad res;
    res.glyphIndex = glyphIndex;
    res.position = pos;
    res.size.x() = glyph.width;
    res.size.y() = glyph.height;
    return res;
}
bool Raster::UpdateMesh(RenderPassParam& param, RenderPassResource& resource)
{
    // create host mesh data
    QuadArrayMesh mesh;
    PackGlyph packGlyph;
    Vec2u frameBufferSize = param.frameBuffer.GetSize();
    packGlyph.height = frameBufferSize.y();
    packGlyph.width = frameBufferSize.x();
    float x;
    float y;
    bool res;
    for (auto unicode : param.unicodes)
    {
        auto& glyph = param.font.glyphs[unicode];
        res = packGlyph.PushQuad(glyph.width, glyph.height, x, y);
        if (!res)
        {
            assert(false && "pack glyph failed");
            return false;
        }
        mesh.PushQuad(CreateQuad(param.font.glyphs[unicode], Vec2f(x, y), unicode));
    }
    // create device mesh data
    if (resource.mesh)
    {
        resource.mesh.Update(mesh.GetMesh());
    }
    else
    {
        resource.mesh = DeviceMesh::Create(mesh.GetMesh());
    }
    return true;
}
bool Raster::UpdateUniformBuffer(RenderPassParam& param, RenderPassResource& resource)
{
    // host
    m_Camera.CalculateMatrix();
    //========
    for (size_t i = 0; i < 16; i++)
    {
        m_HostUniformBuffer.mvp[i] = m_Camera.matrix.data()[i];
    }
    m_HostUniformBuffer.screenSize[0] = m_Camera.screenSize.x();
    m_HostUniformBuffer.screenSize[1] = m_Camera.screenSize.y();
    // stagging
    m_StaggingBuffer.SetData(std::span<uint8_t>((uint8_t*)&m_HostUniformBuffer,
                                                sizeof(m_HostUniformBuffer)));
    // uniform
    return DeviceBuffer::SyncCopy(resource.uniformBuffer, m_StaggingBuffer,
                                  sizeof(m_HostUniformBuffer), 0, 0);
}

bool Raster::UpdateDescriptorSet(RenderPassResource& resource)
{
    auto& descriptorSet = m_DescriptorSet.GetVk();
    auto& uboAccessor = descriptorSet.ubos[0];
    auto& set = descriptorSet.sets[uboAccessor.set];
    vk::DescriptorSetOperator op(set);
    op.BindUBO(uboAccessor.binding, resource.uniformBuffer.GetVk());
    op.Apply();
    return true;
}
bool Raster::RecordCommand(RenderPassParam& param, RenderPassResource& resource)
{
    auto& commandBuffer = param.commandBuffer.GetVk();
    auto& renderPass = param.renderPass.GetVk();
    auto& frameBuffer = param.frameBuffer.GetVk();
    commandBuffer.BindPipeline(m_Pipeline.GetVk());
    auto& descriptorSet = m_DescriptorSet.GetVk();
    for (size_t i = 0; i < descriptorSet.sets.size(); ++i)
    {
        auto& set = descriptorSet.sets[i];
        commandBuffer.BindDescriptorSet(set, m_PipelineLayout.GetVk(), i);
    }
    Render::Utils::DrawMesh(commandBuffer, resource.mesh.GetVk());
    return true;
}
} // namespace Aether::Text