#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/GraphicsCommandPool.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderContext.h"
#include "Render/Vulkan/ShaderModule.h"
#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "Render/Mesh/VkMesh.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
#include "Render/Utils.h"
#include <print>
using namespace Aether;
static const char* vertexShaderCode = R"(
#version 450
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
void main()
{
    gl_Position = vec4(a_Position,  1.0);
}
)";
static const char* fragmentShaderCode = R"(
#version 450
layout(location = 0) out vec4 outColor;
void main()
{
    outColor = vec4(1.0, 1.0, 0.0, 1.0);
}
)";

class TestLayer : public Layer
{
public:
    
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        if (!m_Pipeline)
        {
            // create pipeline
            vk::PipelineLayout::Builder layoutBuilder;
            auto pipelineLayout = layoutBuilder.Build();
            vk::GraphicsPipeline::Builder pipelineBuilder(*m_RenderPass, *pipelineLayout);
            pipelineBuilder.AddFragmentStage(*m_FragmentShader, "main");
            pipelineBuilder.AddVertexStage(*m_VertexShader, "main");
            pipelineBuilder.PushVertexBufferLayouts(m_Mesh->GetVertexBufferLayouts());
            m_Pipeline=pipelineBuilder.BuildScope();
        }
        commandBuffer.BeginRenderPass(*m_RenderPass, framebuffer,Vec4f(1.0f,1.0f,1.0f,1.0f));
        commandBuffer.SetViewport(0, 0, framebuffer.GetSize().width, framebuffer.GetSize().height);
        commandBuffer.SetScissor(0, 0, framebuffer.GetSize().width,framebuffer.GetSize().height);
        commandBuffer.BindPipeline(*m_Pipeline);
        Render::Utils::DrawMesh(commandBuffer, *m_Mesh);
        commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        // create command buffer pool
        m_CommandPool = vk::GraphicsCommandPool::CreateScope();
        // create cpu grid
        auto sphere = Geometry::CreateBox();
        // create gpu grid
        m_Mesh = VkMesh::CreateScope(*m_CommandPool, sphere);
        // compile shader in cpu
        auto vsBin = Shader::Compiler::GLSL2SPIRV(&vertexShaderCode, 1, ShaderStageType::Vertex);
        assert(vsBin);
        std::print("vertex shader size: {}\n", vsBin->size());
        auto fsBin = Shader::Compiler::GLSL2SPIRV(&fragmentShaderCode, 1, ShaderStageType::Fragment);
        assert(fsBin);
        std::print("fragment shader size: {}\n", fsBin->size());
        m_VertexShader = vk::ShaderModule::CreateScopeFromBinaryCode(*vsBin);
        m_FragmentShader = vk::ShaderModule::CreateScopeFromBinaryCode(*fsBin);
        m_RenderPass=CreateScope<vk::RenderPass>(vk::RenderPass::CreateDefault().value());
    }

private:
    Scope<vk::GraphicsPipeline> m_Pipeline;
    Scope<VkMesh> m_Mesh;
    Scope<vk::GraphicsCommandPool> m_CommandPool;
    Scope<vk::DescriptorSet> m_DescriptorSet;
    Scope<vk::ShaderModule> m_VertexShader;
    Scope<vk::ShaderModule> m_FragmentShader;
    Scope<vk::RenderPass> m_RenderPass;
    
};
int main()
{
    WindowContext::Init();
    {
        auto window = std::unique_ptr<Window>(Window::Create(800, 600, "Hello, Aether"));
        // 会在window中创建vulkan对象,在销毁vulkan context前必须调用window 的ReleaseVulkanObjects 销毁window中的vulkan对象
        vk::GRC::Init(window.get(), true);
        auto* testLayer = new TestLayer();
        window->PushLayer(testLayer);
        while (!window->ShouldClose())
        {
            WindowContext::PollEvents();
            window->OnRender();
        }
        vkDeviceWaitIdle(vk::GRC::GetDevice());
        delete testLayer;
        window->ReleaseVulkanObjects();
        vk::GRC::Cleanup();
    }

    WindowContext::Shutdown();
}