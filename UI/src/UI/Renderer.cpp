#include "Renderer.h"
#include "Core/Math/Utils.h"
#include "Render/Config.h"
#include "Render/RenderApi/DeviceBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceMesh.h"
#include "Render/Shader/ShaderSource.h"
#include "Render/Utils.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/DynamicDescriptorPool.h"
#include "Render/Vulkan/FrameBuffer.h"
#include "Render/Vulkan/GraphicsCommandBuffer.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderPass.h"
#include <cassert>
#include "QuadArrayMesh.h"
#include "Render/Basis.h"
namespace Aether::UI
{

bool Renderer::CreateBasicPipeline(DeviceRenderPassView _renderPass)
{
    if (Render::Config::RenderApi != Render::Api::Vulkan)
    {
        assert(false && "not implemented");
    }
    auto& vkDescriptorSet = m_BasicDescriptorSet.GetVk();
    vk::PipelineLayout::Builder layoutBuilder;
    for (auto& layout : vkDescriptorSet.layouts)
    {
        layoutBuilder.AddDescriptorSetLayout(layout);
    }
    auto layout = layoutBuilder.Build();
    if (!layout)
    {
        return false;
    }

    auto dummyMesh = QuadArrayMesh();
    auto vertextLayouts = dummyMesh.GetMesh().CreateVertexBufferLayouts();
    auto& renderPass = _renderPass.Get<vk::RenderPass>();
    auto& shader = m_BasicShader.GetVk();
    vk::GraphicsPipeline::Builder builder(renderPass, layout.value());
    builder.PushVertexBufferLayouts(vertextLayouts);
    builder.AddVertexStage(shader.vertex, "main");
    builder.AddFragmentStage(shader.fragment, "main");
    auto pipeline = builder.Build();
    if (!pipeline)
    {
        return false;
    }
    m_BasicPipeline = std::move(pipeline.value());
    m_BasicPipelineLayout = std::move(layout.value());
    return true;
}
void Renderer::Begin(DeviceRenderPassView renderPass, DeviceFrameBufferView frameBuffer, Vec2f screenSize)
{
    m_FrameBuffer = frameBuffer;
    m_RenderPass = renderPass;
    m_ScreenSize = screenSize;
    // clear quad mesh
    m_BasicQuadArray = QuadArrayMesh(); // clear basic quads
    m_QuadsWithTexture.clear();         // clear quads with texture
    m_Temporary.ClearAll();             // clear temporary(e.g. device mesh in last frame)
}
void Renderer::DrawQuad(const Quad& quad)
{
    if (!quad.GetShader() && !quad.GetTexture())
    {
        m_BasicQuadArray.PushQuad(quad);
    }
    else if (!quad.GetShader() && quad.GetTexture())
    {
        // has same texture yet?
        for (auto& q : m_QuadsWithTexture)
        {
            if (q.texture == quad.GetTexture())
            {
                q.mesh.PushQuad(quad);
                return;
            }
        }
        // create new
        QuadWithTexture qwt;
        qwt.texture = quad.GetTexture();
        qwt.mesh.PushQuad(quad);
        m_QuadsWithTexture.push_back(qwt);
    }
    else
    {
        assert(false && "not implemented");
    }
}
void Renderer::End(DeviceCommandBufferView _commandBuffer)
{
    // basic
    {
        CreateBasicDescriptorSet();
        // update uniform
        {
            auto modelMatrix = CalculateModelMatrix();
            m_BasicUboLocalBuffer.model = modelMatrix;
            m_BasicUboLocalBuffer.view = Mat4::Identity();
            UploadBasicBuffer();
        }
        auto& descriptorResource = m_BasicDescriptorSet.GetVk();

        // record command in input command buffer
        auto& commandBuffer = _commandBuffer.Get<vk::GraphicsCommandBuffer>();
        auto& renderPass = m_RenderPass.Get<vk::RenderPass>();
        auto& frameBuffer = m_FrameBuffer.Get<vk::FrameBuffer>();
        auto& pipeline = m_BasicPipeline.GetVk();
        auto& pipelineLayout = m_BasicPipelineLayout.GetVk();
        auto* meshp = DeviceMesh::CreateRaw(m_BasicQuadArray.GetMesh());
        assert(meshp && "failed to create mesh");
        m_Temporary.Push(meshp);
        auto& mesh = (*meshp).Get<VkMesh>();

        // commandBuffer.BeginRenderPass(renderPass, frameBuffer);
        commandBuffer.BindPipeline(pipeline);
        commandBuffer.SetViewport(0, 0, m_ScreenSize.x(), m_ScreenSize.y());
        commandBuffer.SetScissor(0, 0, m_ScreenSize.x(), m_ScreenSize.y());
        for (size_t i = 0; i < descriptorResource.sets.size(); ++i)
        {
            auto& set = descriptorResource.sets[i];
            commandBuffer.BindDescriptorSet(set, pipelineLayout, 0);
        }
        Render::Utils::DrawMesh(commandBuffer, mesh);
        // commandBuffer.EndRenderPass();
    }
}
std::expected<Renderer, std::string> Renderer::Create(DeviceRenderPassView renderPass, const RenderResource& resource)
{
    Renderer renderer;
    renderer.m_RenderResource = resource;
    // basic
    if (!renderer.CreateBasicShader())
    {
        return std::unexpected<std::string>("Failed to create basic shader");
    }
    if (!renderer.CreateBasicDescriptorSet())
    {
        return std::unexpected<std::string>("Failed to create basic quad array");
    }
    if (!renderer.CreateBasicPipeline(renderPass))
    {
        return std::unexpected<std::string>("Failed to create basic pipeline");
    }
    if (!renderer.CreateBasicBuffer())
    {
        return std::unexpected<std::string>("Failed to create basic buffer");
    }
    // texture
    if(!renderer.CreateQuadWithTextureShader())
    {
        return std::unexpected<std::string>("Failed to create quad with texture shader");
    }
    if (!renderer.CreateQuadWithTextureShader())
    {
        return std::unexpected<std::string>("Failed to create quad with texture shader");
    }
    if (!renderer.CreateQuadWithTextureDescriptorSet())
    {
        return std::unexpected<std::string>("Failed to create quad with texture descriptor set");
    }
    return renderer;
}
bool Renderer::CreateBasicDescriptorSet()
{
    auto& descriptorPool = *m_RenderResource.m_DescriptorPool;
    m_BasicDescriptorSet = descriptorPool.CreateSet(1, 0, 0);
    return true;
}
bool Renderer::CreateBasicShader()
{
    static const char* frag = R"(
#version 450
layout(location=0)in vec4 v_Color;
layout(location=0)out vec4 FragColor;
void main()
{
    FragColor=v_Color;
}
)";
    static const char* vert = R"(
#version 450
layout(location=0)in vec2 a_Position;
layout(location=1)in vec2 a_UV;
layout(location=2)in vec4 a_Color;
layout(location=0)out vec4 v_Color;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_Model;
    mat4 u_View;
}ubo;
void main()
{
    vec4 pos=vec4(a_Position,0.0,1.0);

    gl_Position=ubo.u_View*ubo.u_Model*pos;
    v_Color=a_Color;
}
)";
    auto shaderEx = DeviceShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vert),
                                         ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, frag));
    if (!shaderEx)
    {
        return false;
    }
    m_BasicShader = std::move(shaderEx.value());
    return true;
}
Mat4 Renderer::CalculateModelMatrix()
{
    Mat4 m = Mat4::Identity();
    // normalize screen space to [0,1]
    m = Math::Scale(Vec3f(1 / m_ScreenSize.x(), 1 / m_ScreenSize.y(), 1)) * m;
    Mat4 m1 = CalculateBasisTransform(GetNormalizedScreenBasis(), GetNdcBasis());
    // translate screen coord to NDC space coord
    m = m1 * m;
    return m;
}
Renderer Renderer::CreateEmpty()
{
    return Renderer();
}
bool Renderer::CreateBasicBuffer()
{
    m_BasicUboBuffer = DeviceBuffer::CreateForUniform(sizeof(BasicUniformBlock));
    if (m_BasicUboBuffer.Empty())
    {
        return false;
    }

    return true;
}
bool Renderer::UploadBasicBuffer()
{
    // get descriptor
    auto& descriptorResource = m_BasicDescriptorSet.GetVk();
    auto& uboAccessor = descriptorResource.ubos[0];
    auto& set = descriptorResource.sets[uboAccessor.set];
    auto binding = uboAccessor.binding;
    auto& uboBuffer = m_BasicUboBuffer.Get<vk::Buffer>();
    // bind buffer
    vk::DescriptorSetOperator op(set);
    op.BindUBO(binding, uboBuffer);
    op.Apply();
    // update data
    uboBuffer.SetData((const uint8_t*)&m_BasicUboLocalBuffer, sizeof(BasicUniformBlock));
    return true;
}
bool Renderer::CreateQuadWithTextureShader()
{
    static const char* frag = R"(
#version 450
layout(location=0)in vec4 v_Color;
layout(location=0)out vec4 FragColor;
layout(location=1)uniform sampler2D u_Texture;
layout(location=2) in vec2 v_UV;
void main()
{
    FragColor=texture(u_Texture,v_UV);
}
)";
    static const char* vert = R"(
#version 450
layout(location=0)in vec2 a_Position;
layout(location=1)in vec2 a_UV;
layout(location=2)in vec4 a_Color;
layout(location=0)out vec4 v_Color;
layout(location=1)out vec2 v_UV;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_Model;
    mat4 u_View;
}ubo;
void main()
{
    vec4 pos=vec4(a_Position,0.0,1.0);

    gl_Position=ubo.u_View*ubo.u_Model*pos;
    v_UV=a_UV;
    v_Color=a_Color;
}
)";
    return true;
}
bool Renderer::CreateQuadWithTextureDescriptorSet()
{
    auto& descriptorPool = *m_RenderResource.m_DescriptorPool;
    m_QuadWithTextureDescriptorSet = descriptorPool.CreateSet(1, 0, 1);
    return true;
}
bool Renderer::CreateQuadWithTexturePipeline(DeviceRenderPassView _renderPass)
{
    assert(Render::Config::RenderApi == Render::Api::Vulkan && "only support vulkan now");
    // pipeline layout
    auto& vkDescriptorSet = m_QuadWithTextureDescriptorSet.GetVk();
    vk::PipelineLayout::Builder layoutBuilder;
    for (auto& layout : vkDescriptorSet.layouts)
    {
        layoutBuilder.AddDescriptorSetLayout(layout);
    }
    auto layout = layoutBuilder.Build();
    if (!layout)
    {
        return false;
    }
    // pipeline
    auto dummyMesh = QuadArrayMesh();
    auto vertextLayouts = dummyMesh.GetMesh().CreateVertexBufferLayouts();
    auto& renderPass = _renderPass.Get<vk::RenderPass>();
    auto& shader = m_QuadWithTextureShader.GetVk();
    vk::GraphicsPipeline::Builder builder(renderPass, layout.value());
    builder.PushVertexBufferLayouts(vertextLayouts);
    builder.AddVertexStage(shader.vertex, "main");
    builder.AddFragmentStage(shader.fragment, "main");
    auto pipeline = builder.Build();
    if (!pipeline)
    {
        return false;
    }
    m_QuadWithTexturePipeline = std::move(pipeline.value());
    m_QuadWithTexturePipelineLayout = std::move(layout.value());
    return true;
}
bool Renderer::CreateQuadWithTextureSampler()
{
    assert(Render::Api::Vulkan == Render::Config::RenderApi && "only support vulkan now");
    vk::Sampler::Builder builder;
    builder.SetMagFilter(vk::Sampler::Filter::Linear);
    builder.SetMinFilter(vk::Sampler::Filter::Linear);
    builder.SetMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    builder.SetAddressMode(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
    builder.SetAnisotropyEnable(true);
    builder.SetMaxAnisotropy(16);
    auto sampler = builder.Build();
    if (!sampler)
    {
        return false;
    }
    m_QuadWithTextureSampler = std::move(sampler.value());
    return true;
}
} // namespace Aether::UI