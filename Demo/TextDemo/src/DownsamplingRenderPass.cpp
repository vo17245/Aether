#include "DownsamplingRenderPass.h"
#include <Render/Mesh/Geometry.h>
#include <Render/Utils.h>
std::optional<DownsamplingRenderPass> DownsamplingRenderPass::Create(DeviceDescriptorPool& pool)
{
    static const char* vs=R"(
    #version 450
layout(location=0)in vec2 a_Position;
layout(location=1)in vec2 a_UV;
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
    vec4 color = BoxFilter(u_Texture, v_UV);
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
    DownsamplingRenderPass pass;
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
    // create descriptor set layout
    auto set=pool.CreateSet(0, 0, 1);
    // create sampler
    pass.m_Sampler=DeviceSampler::CreateNearest();
    // create pipeline layout
    vk::PipelineLayout::Builder layoutBuilder;

    pass.m_PipelineLayout=layoutBuilder.AddDescriptorSetLayouts(set.GetVk().layouts).Build().value();
    // create pipeline
    vk::GraphicsPipeline::Builder builder(renderPass.GetVk(),pass.m_PipelineLayout.GetVk());
    builder.PushVertexBufferLayouts(mesh.CreateVertexBufferLayouts());
    builder.AddVertexStage(shader.GetVk().vertex,"main");
    builder.AddFragmentStage(shader.GetVk().fragment,"main");
    pass.m_Pipeline=builder.Build().value();
    return pass;

}
bool DownsamplingRenderPass::Render(DeviceCommandBufferView& commandBuffer,DeviceTexture& texture,DeviceDescriptorPool& pool)
{
    auto& cb=commandBuffer.GetVk();
    // create and update descriptor set
    auto set=pool.CreateSet(0, 0, 1);
    assert(set);
    auto& vkSet=set.GetVk();
    for(auto& sampler:set.GetVk().samplers)
    {
        auto& s=vkSet.sets[sampler.set];
        vk::DescriptorSetOperator op(s);
        op.BindSampler(sampler.binding, m_Sampler.GetVk(), texture.GetOrCreateDefaultImageView().GetVk());
        op.Apply();
    }

    // bind pipeline
    cb.BindPipeline(m_Pipeline.GetVk());
    for(size_t i=0;i<vkSet.sets.size();++i)
    {
        auto& s=vkSet.sets[i];
        cb.BindDescriptorSet(s, m_PipelineLayout.GetVk(), i);
    }
    Render::Utils::DrawMesh(cb, m_Mesh.GetVk());
    return true;
}