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
bool Renderer::CreateDescriptorPool()
{
    if (Render::Config::RenderApi != Render::Api::Vulkan)
    {
        assert(false && "not implemented");
    }
    auto poolEx = DeviceDescriptorPool::Create();
    if (!poolEx)
    {
        return false;
    }
    m_DescriptorPool = std::move(poolEx.value());
    return true;
}
bool Renderer::CreateBasicPipeline(DeviceRenderPassView _renderPass)
{
    if (Render::Config::RenderApi != Render::Api::Vulkan)
    {
        assert(false && "not implemented");
    }
    auto& vkDescriptorSet = std::get<vk::DynamicDescriptorPool::DescriptorResource>(m_BasicDescriptorSet);
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
    auto& shader = std::get<VulkanShader>(m_BasicShader);
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
    m_BasicQuadArray = QuadArrayMesh();
    m_Temporary.ClearAll();
}
void Renderer::DrawQuad(const Quad& quad)
{
    if (!quad.GetShader() && !quad.GetTexture())
    {
        m_BasicQuadArray.PushQuad(quad);
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
        //update uniform
        {
            auto modelMatrix=CalculateModelMatrix();
            m_BasicUboLocalBuffer.model=modelMatrix;
            m_BasicUboLocalBuffer.view=Mat4::Identity();
            UploadBasicBuffer();
        }
        auto& descriptorResource = std::get<vk::DynamicDescriptorPool::DescriptorResource>(m_BasicDescriptorSet);
        

        // record command in input command buffer
        auto& commandBuffer = _commandBuffer.Get<vk::GraphicsCommandBuffer>();
        auto& renderPass = m_RenderPass.Get<vk::RenderPass>();
        auto& frameBuffer = m_FrameBuffer.Get<vk::FrameBuffer>();
        auto& pipeline = std::get<vk::GraphicsPipeline>(m_BasicPipeline);
        auto& pipelineLayout = std::get<vk::PipelineLayout>(m_BasicPipelineLayout);
        auto* meshp=DeviceMesh::CreateRaw(m_BasicQuadArray.GetMesh());
        assert(meshp && "failed to create mesh");
        m_Temporary.Push(meshp);
        auto& mesh=(*meshp).Get<VkMesh>();

        //commandBuffer.BeginRenderPass(renderPass, frameBuffer);
        commandBuffer.BindPipeline(pipeline);
        commandBuffer.SetViewport(0, 0, m_ScreenSize.x(), m_ScreenSize.y());
        commandBuffer.SetScissor(0, 0, m_ScreenSize.x(), m_ScreenSize.y());
        for (size_t i = 0; i < descriptorResource.sets.size(); ++i)
        {
            auto& set = descriptorResource.sets[i];
            commandBuffer.BindDescriptorSet(set, pipelineLayout, 0);
        }
        Render::Utils::DrawMesh(commandBuffer, mesh);
        //commandBuffer.EndRenderPass();
    }
}
std::expected<Renderer, std::string> Renderer::Create(DeviceRenderPassView renderPass)
{
    Renderer renderer;
    if (!renderer.CreateDescriptorPool())
    {
        return std::unexpected<std::string>("Failed to create descriptor pool");
    }
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
    if(!renderer.CreateBasicBuffer())
    {
        return std::unexpected<std::string>("Failed to create basic buffer");
    }
    return renderer;
}
bool Renderer::CreateBasicDescriptorSet()
{
    m_BasicDescriptorSet = m_DescriptorPool.CreateSet(1, 0, 0);
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
    auto shaderEx = CreateDeviceShader(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vert),
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
    m_BasicUboBuffer=DeviceBuffer::CreateForUniform(sizeof(BasicUniformBlock));
    if(m_BasicUboBuffer.Empty())
    {
        return false;
    }
    m_BasicUboStaging=DeviceBuffer::CreateForStaging(sizeof(BasicUniformBlock));
    if(m_BasicUboStaging.Empty())
    {
        return false;
    }
    return true;
}
bool Renderer::UploadBasicBuffer()
{
    // get descriptor
    auto& descriptorResource=std::get<vk::DynamicDescriptorPool::DescriptorResource>(m_BasicDescriptorSet);
    auto& uboAccessor=descriptorResource.ubos[0];
    auto& set=descriptorResource.sets[uboAccessor.set];
    auto binding=uboAccessor.binding;
    auto& uboBuffer=m_BasicUboBuffer.Get<vk::Buffer>();
    // bind buffer
    vk::DescriptorSetOperator op(set);
    op.BindUBO(binding, uboBuffer);
    op.Apply();
    // update data
    uboBuffer.SetData((const uint8_t*)&m_BasicUboLocalBuffer, sizeof(BasicUniformBlock));
    return true;
}
} // namespace Aether::UI