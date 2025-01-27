#include "Renderer.h"
#include "Render/Config.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/Shader/ShaderSource.h"
#include "Render/Utils.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderPass.h"
#include <cassert>
#include "QuadArrayMesh.h"
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
bool Renderer::CreateBasicPipeline(DeviceRenderPass& _renderPass)
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
    auto& renderPass = std::get<vk::RenderPass>(_renderPass);
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
    return true;
}
void Renderer::Begin(DeviceRenderPass& renderPass, DeviceFrameBuffer& frameBuffer)
{
    m_FrameBuffer = &frameBuffer;
    m_RenderPass = &renderPass;
    // clear quad mesh
    m_BasicQuadArray = QuadArrayMesh();
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
void Renderer::End(DeviceCommandBuffer& _commandBuffer)
{
    // basic
    {
        auto& commandBuffer = std::get<vk::GraphicsCommandBuffer>(_commandBuffer);
        auto& renderPass = std::get<vk::RenderPass>(*m_RenderPass);
        auto& frameBuffer = std::get<vk::FrameBuffer>(*m_FrameBuffer);
        auto& pipeline = std::get<vk::GraphicsPipeline>(m_BasicPipeline);
        auto mesh = VkMesh::Create(vk::GRC::GetGraphicsCommandPool(), m_BasicQuadArray.GetMesh());
        commandBuffer.BeginRenderPass(renderPass, frameBuffer);
        commandBuffer.BindPipeline(pipeline);
        Render::Utils::DrawMesh(commandBuffer, mesh.value());
        commandBuffer.EndRenderPass();
    }
}
std::expected<Renderer, std::string> Renderer::Create(DeviceRenderPass& renderPass)
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
void main()
{
    gl_Position=vec4(a_Position,0.0,1.0);
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
} // namespace Aether::UI