#include "DownsamplerRenderPass.h"
#include <Render/Mesh/Geometry.h>
#include <Render/Utils.h>
std::optional<DownsamplerRenderPass> DownsamplerRenderPass::Create()
{
    static const char* vs=R"(
    #version 450
layout(location=0)in vec2 a_Position;
layout(location=2)in vec2 a_UV;
layout(location=0)out vec2 v_UV;
void main()
{
    vec4 pos=vec4(a_Position,0.0,1.0);
    gl_Position=pos;
    v_UV=a_UV;
}
    )";
    static const char* fs=R"(
layout(location=0)in vec2 v_UV;
layout(location=0)out vec4 FragColor;
layout(set=0,binding=0)uniform sampler2D u_Texture;

vec4 BoxFilter(sampler2D tex, vec2 uv) {
    vec2 texelSize = 1.0 / textureSize(tex, 0);
    vec4 sum = vec4(0.0);
    for (int x = -1; x <= 2; ++x) {
        for (int y = -1; y <= 2; ++y) {
            sum += texture(tex, uv + texelSize * vec2(x, y));
        }
    }
    return sum / 16.0;
}
void main()
{
    Vec4 color = BoxFilter(u_Texture, v_UV);
    FragColor = color;
}
    )";
    auto shaderEx = DeviceShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vs),
                                         ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, fs));
    if (!shaderEx)
    {
        assert(false && "create shader failed");
        return std::nullopt;
    }
    auto& shader = shaderEx.value();
    DownsamplerRenderPass pass;
    // create mesh
    auto mesh=Geometry::CreateQuad();
    pass.m_Mesh=DeviceMesh::Create(mesh);
    // create a dummy render pass
    DeviceRenderPassDesc renderPassDesc;
    renderPassDesc.colorAttachmentCount=1;
    renderPassDesc.colorAttachments[0].format=PixelFormat::RGBA8888;
    renderPassDesc.colorAttachments[0].loadOp=DeviceAttachmentLoadOp::Clear;
    renderPassDesc.colorAttachments[0].storeOp=DeviceAttachmentStoreOp::Store;
    auto renderPass = DeviceRenderPass::Create(renderPassDesc);
    if (renderPass.Empty())
    {
        assert(false && "create render pass failed");
        return std::nullopt;
    }
    vk::PipelineLayout::Builder layoutBuilder;
    pass.m_PipelineLayout=layoutBuilder.Build().value();
    vk::GraphicsPipeline::Builder builder(renderPass.GetVk(),pass.m_PipelineLayout.GetVk());
    builder.PushVertexBufferLayouts(mesh.CreateVertexBufferLayouts());
    builder.AddVertexStage(shader.GetVk().vertex,"main");
    builder.AddFragmentStage(shader.GetVk().fragment,"main");
    pass.m_Pipeline=builder.Build().value();
    return pass;

}
bool DownsamplerRenderPass::Render(DeviceCommandBufferView& commandBuffer,DeviceTexture& texture)
{
    auto& cb=commandBuffer.GetVk();
    cb.BindPipeline(m_Pipeline.GetVk());
    Render::Utils::DrawMesh(cb, m_Mesh.GetVk());
}