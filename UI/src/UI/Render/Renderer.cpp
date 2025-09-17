#include "Renderer.h"
#include "Core/Math/Utils.h"
#include "Render/Config.h"
#include "Render/RenderApi/DeviceBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceMesh.h"
#include "Render/Scene/Camera2D.h"
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
    builder.EnableDepthTest();
    builder.EnableBlend();
    auto pipeline = builder.Build();
    if (!pipeline)
    {
        return false;
    }
    m_BasicPipeline = std::move(pipeline.value());
    m_BasicPipelineLayout = std::move(layout.value());
    return true;
}
void Renderer::Begin(DeviceFrameBufferView frameBuffer, const Camera2D& camera)
{
    #ifdef AETHER_RUNTIME_CHECK
    assert(m_IsBusy==false &&" renderer is busy");
    #endif
    Clear();
    m_IsBusy=true;
    m_Camera=camera;
    m_FrameBuffer = frameBuffer;
    
}
void Renderer::Reset()
{
    m_IsBusy = false;
    Clear();
}
void Renderer::Clear()
{
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
        qwt.descriptorSet = CreateQuadWithTextureDescriptorSet();
        qwt.sampler = m_SamplerPool.GetOrCreate();
        m_QuadsWithTexture.push_back(std::move(qwt));
    }
    else
    {
        assert(false && "not implemented");
    }
}
void Renderer::End(DeviceCommandBufferView _commandBuffer)
{
    // basic
    if(m_BasicQuadArray.GetMesh().CalculateVertexCount())
    {

        CreateBasicDescriptorSet();
        // update uniform
        {
            auto& modelMatrix = m_Camera.matrix;
            m_BasicUboLocalBuffer.model = modelMatrix;
            m_BasicUboLocalBuffer.view = Mat4f::Identity();
            UploadBasicBuffer();
        }
        auto& descriptorResource = m_BasicDescriptorSet.GetVk();

        // record command in input command buffer
        auto& commandBuffer = _commandBuffer.Get<vk::GraphicsCommandBuffer>();
        auto& frameBuffer = m_FrameBuffer.Get<vk::FrameBuffer>();
        auto& pipeline = m_BasicPipeline.GetVk();
        auto& pipelineLayout = m_BasicPipelineLayout.GetVk();
        auto* meshp = DeviceMesh::CreateRaw(m_BasicQuadArray.GetMesh());
        assert(meshp && "failed to create mesh");
        m_Temporary.Push(meshp);
        auto& mesh = (*meshp).Get<VkMesh>();

        commandBuffer.BindPipeline(pipeline);
        auto& screenSize = m_Camera.screenSize;
        commandBuffer.SetViewport(0, 0, screenSize.x(), screenSize.y());
        commandBuffer.SetScissor(0, 0, screenSize.x(), screenSize.y());
        for (size_t i = 0; i < descriptorResource.sets.size(); ++i)
        {
            auto& set = descriptorResource.sets[i];
            commandBuffer.BindDescriptorSet(set, pipelineLayout, 0);
        }
        Render::Utils::VkDrawMesh(commandBuffer, mesh);
    }
    // quad with texture
    {
        UpdateQuadWithTextureDeviceUniformBuffer();
        for (auto& qwt : m_QuadsWithTexture)
        {
            UpdateQuadWithTextureDescriptor(qwt);
            auto& descriptorResource = qwt.descriptorSet.GetVk();
            auto& commandBuffer = _commandBuffer.Get<vk::GraphicsCommandBuffer>();
            auto& frameBuffer = m_FrameBuffer.Get<vk::FrameBuffer>();
            auto& pipeline = m_QuadWithTexturePipeline.GetVk();
            auto& pipelineLayout = m_QuadWithTexturePipelineLayout.GetVk();
            auto* meshp = DeviceMesh::CreateRaw(qwt.mesh.GetMesh());
            assert(meshp && "failed to create mesh");
            m_Temporary.Push(meshp);
            auto& mesh = (*meshp).Get<VkMesh>();

            commandBuffer.BindPipeline(pipeline);
            auto& screenSize = m_Camera.screenSize;
            commandBuffer.SetViewport(0, 0, screenSize.x(), screenSize.y());
            commandBuffer.SetScissor(0, 0, screenSize.x(), screenSize.y());
            for (size_t i = 0; i < descriptorResource.sets.size(); ++i)
            {
                auto& set = descriptorResource.sets[i];
                commandBuffer.BindDescriptorSet(set, pipelineLayout, i);
            }
            Render::Utils::VkDrawMesh(commandBuffer, mesh);
        }
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
    if (!renderer.CreateQuadWithTextureShader())
    {
        return std::unexpected<std::string>("Failed to create quad with texture shader");
    }
    if (!renderer.CreateQuadWithTextureDescriptorSet())
    {
        return std::unexpected<std::string>("Failed to create quad with texture descriptor set");
    }
    if (!renderer.CreateQuadWithTexturePipeline(renderPass))
    {
        return std::unexpected<std::string>("Failed to create quad with texture pipeline");
    }
    if(!renderer.CreateQuadWithTextureBuffer())
    {
        return std::unexpected<std::string>("Failed to create quad with texture uniform buffer");
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
layout(location=0)in vec3 a_Position;
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
    vec4 pos=vec4(a_Position,1.0);

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
layout(set=1,binding=0)uniform sampler2D u_Texture;
layout(location=1) in vec2 v_UV;


void main()
{
    FragColor=texture(u_Texture,v_UV);
    //FragColor=vec4(v_UV,1.0,1.0);//debug
}
)";
    static const char* vert = R"(
#version 450
layout(location=0)in vec3 a_Position;
layout(location=1)in vec2 a_UV;
layout(location=2)in vec4 a_Color;
layout(location=0)out vec4 v_Color;
layout(location=1)out vec2 v_UV;
layout(std140,set=0,binding=0)uniform UniformBufferObject
{
    mat4 u_Model;
    mat4 u_View;
    mat4 u_TexCoord;
}ubo;
mat3 ExtractMat3(mat4 m) {
    return mat3(m[0].xyz, m[1].xyz, m[2].xyz);
}
void main()
{
    vec4 pos=vec4(a_Position,1.0);
    mat3 texCoordMatrix=ExtractMat3(ubo.u_TexCoord);
    gl_Position=ubo.u_View*ubo.u_Model*pos;
    v_UV=(texCoordMatrix*vec3(a_UV,1.0)).xy;
    v_Color=a_Color;
}
)";
    auto shaderEx = DeviceShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vert),
                                         ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, frag));
    if (!shaderEx)
    {
        return false;
    }
    m_QuadWithTextureShader = std::move(shaderEx.value());
    return true;
}
DeviceDescriptorSet Renderer::CreateQuadWithTextureDescriptorSet()
{
    auto& descriptorPool = *m_RenderResource.m_DescriptorPool;
    auto set = descriptorPool.CreateSet(1, 0, 1);
    return set;
}
bool Renderer::CreateQuadWithTexturePipeline(DeviceRenderPassView _renderPass)
{
    assert(Render::Config::RenderApi == Render::Api::Vulkan && "only support vulkan now");
    // pipeline layout
    auto descriptorSet = CreateQuadWithTextureDescriptorSet();
    auto& vkDescriptorSet = descriptorSet.GetVk();
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
    builder.EnableDepthTest();
    builder.EnableBlend();
    auto pipeline = builder.Build();
    if (!pipeline)
    {
        return false;
    }
    m_QuadWithTexturePipeline = std::move(pipeline.value());
    m_QuadWithTexturePipelineLayout = std::move(layout.value());
    return true;
}
DeviceSampler Renderer::CreateQuadWithTextureSampler()
{
    // create new
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
        return {};
    }
    return std::move(sampler.value());
}
bool Renderer::UpdateQuadWithTextureDescriptor(QuadWithTexture& quad)
{
    // update device uniform buffer
    m_QuadWithTextureUboBuffer.SetData((const uint8_t*)&m_QuadWithTextureUboLocalBuffer, sizeof(QuadWithTextureUniformBlock));
    // update descriptor
    auto& descriptorResource = quad.descriptorSet.GetVk();
    // ubo
    {
        auto& accessor = descriptorResource.ubos[0];
        auto& set = descriptorResource.sets[accessor.set];
        auto binding = accessor.binding;
        vk::DescriptorSetOperator op(set);
        op.BindUBO(binding, m_QuadWithTextureUboBuffer.Get<vk::Buffer>());
        op.Apply();
    }
    // sampler
    {
        auto& accessor = descriptorResource.samplers[0];
        auto& set = descriptorResource.sets[accessor.set];
        auto binding = accessor.binding;
        vk::DescriptorSetOperator op(set);
        op.BindSampler(binding, quad.sampler.Get().GetVk(), quad.texture->GetOrCreateDefaultImageView().GetVk());
        op.Apply();
    }
    return true;
}
bool Renderer::UpdateQuadWithTextureDeviceUniformBuffer()
{
    // update local uniform buffer
    m_QuadWithTextureUboLocalBuffer.model = m_Camera.matrix;
    m_QuadWithTextureUboLocalBuffer.view = Mat4f::Identity();
    m_QuadWithTextureUboLocalBuffer.texCoord = Mat4f::Identity();
    return true;
}
bool Renderer::CreateQuadWithTextureBuffer()
{
    m_QuadWithTextureUboBuffer = DeviceBuffer::CreateForUniform(sizeof(QuadWithTextureUniformBlock));
    if (m_QuadWithTextureUboBuffer.Empty())
    {
        return false;
    }
    return true;
}
} // namespace Aether::UI