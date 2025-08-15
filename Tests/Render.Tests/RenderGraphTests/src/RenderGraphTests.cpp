#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Render/Mesh/VkMesh.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
#include "Render/Utils.h"
#include <print>
#include "Entry/Application.h"
#include <Render/RenderGraph/RenderGraph.h>
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
    ~TestLayer()
    {
        m_RenderGraph.reset();
        m_ResourceLruPool.reset();
    }
    virtual void OnRender(DeviceCommandBuffer& _commandBuffer) override
    {
        m_RenderGraph->SetCommandBuffer(&_commandBuffer);
        m_RenderGraph->Execute();
        // auto& commandBuffer = _commandBuffer.GetVk();
        // commandBuffer.BeginRenderPass(m_RenderPass.GetVk(), m_Framebuffer.GetVk(), Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
        // commandBuffer.SetViewport(0, 0, m_Framebuffer.GetSize().x(), m_Framebuffer.GetSize().y());
        // commandBuffer.SetScissor(0, 0, m_Framebuffer.GetSize().x(), m_Framebuffer.GetSize().y());
        // commandBuffer.BindPipeline(*m_Pipeline);
        // Render::Utils::DrawMesh(commandBuffer, *m_Mesh);
        // commandBuffer.EndRenderPass();
    }
    virtual void OnAttach(Window* window) override
    {
        // create command buffer pool
        m_CommandPool = vk::GraphicsCommandPool::CreateScope();
        //// create cpu grid
        auto sphere = Geometry::CreateBox();
        //// create gpu grid
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

        {
            DeviceRenderPassDesc desc;
            desc.colorAttachmentCount = 1;
            desc.colorAttachments[0].format = PixelFormat::RGBA8888;
            desc.colorAttachments[0].loadOp = DeviceAttachmentLoadOp::Clear;
            desc.colorAttachments[0].storeOp = DeviceAttachmentStoreOp::Store;
            m_RenderPass = DeviceRenderPass::Create(desc);
        }
        {
            DeviceFrameBufferDesc desc;
            desc.colorAttachmentCount = 1;
            desc.colorAttachments[0] = &window->GetFinalTexture().GetOrCreateDefaultImageView();
            desc.width = window->GetSize().x();
            desc.height = window->GetSize().y();
            m_Framebuffer = DeviceFrameBuffer::Create(m_RenderPass, desc);
        }
        {
            // create pipeline
            vk::PipelineLayout::Builder layoutBuilder;
            auto pipelineLayout = layoutBuilder.Build();
            vk::GraphicsPipeline::Builder pipelineBuilder(m_RenderPass.GetVk(), *pipelineLayout);
            pipelineBuilder.AddFragmentStage(*m_FragmentShader, "main");
            pipelineBuilder.AddVertexStage(*m_VertexShader, "main");
            pipelineBuilder.PushVertexBufferLayouts(m_Mesh->GetVertexBufferLayouts());
            m_Pipeline = pipelineBuilder.BuildScope();
        }
        { // build RenderGraph
            // create render graph
            {
                m_ResourceArena = CreateScope<RenderGraph::ResourceArena>();
                m_ResourceLruPool = CreateScope<RenderGraph::ResourceLruPool>(*m_ResourceArena);
                m_RenderGraph = CreateScope<RenderGraph::RenderGraph>(m_ResourceArena.get(), m_ResourceLruPool.get());
            }
            RenderGraph::AccessId<DeviceTexture> finalTextureId;
            // import final texture
            {
                auto& finalTexture = window->GetFinalTexture();
                auto resourceId = m_ResourceArena->Import(&finalTexture);
                auto& accessor = m_RenderGraph->GetResourceAccessor();
                auto& slot = accessor.CreateExternalSlot<DeviceTexture>(resourceId);
                finalTextureId = slot.id;
            }
            // create render task
            {
                struct TaskData
                {
                };
                Vec2i size = window->GetSize();
                auto taskData = m_RenderGraph->AddRenderTask<TaskData>(
                    [&](RenderGraph::RenderTaskBuilder& builder, TaskData& data) {
                        auto imageView = builder.Create<DeviceImageView>(
                            RenderGraph::ImageViewDesc{.texture = finalTextureId, .desc = {}});
                        RenderGraph::RenderPassDesc renderPassDesc;
                        renderPassDesc.colorAttachmentCount = 1;
                        renderPassDesc.colorAttachment[0].imageView = imageView;
                        builder.SetRenderPassDesc(renderPassDesc);
                    },
                    [size, this](DeviceCommandBuffer& _commandBuffer, RenderGraph::ResourceAccessor& accessor,
                                 TaskData& data) {
                        auto& commandBuffer = _commandBuffer.GetVk();
                        commandBuffer.SetViewport(0, 0, size.x(), size.y());
                        commandBuffer.SetScissor(0, 0, size.x(), size.y());
                        commandBuffer.BindPipeline(*m_Pipeline);
                        Render::Utils::DrawMesh(commandBuffer, *m_Mesh);
                    });
            }
            m_RenderGraph->Compile();
        }
    }

private:
    Scope<vk::GraphicsPipeline> m_Pipeline;
    Scope<VkMesh> m_Mesh;
    Scope<vk::GraphicsCommandPool> m_CommandPool;
    Scope<vk::DescriptorSet> m_DescriptorSet;
    Scope<vk::ShaderModule> m_VertexShader;
    Scope<vk::ShaderModule> m_FragmentShader;
    DeviceRenderPass m_RenderPass;
    DeviceFrameBuffer m_Framebuffer;

private:
    Scope<RenderGraph::RenderGraph> m_RenderGraph;
    Scope<RenderGraph::ResourceArena> m_ResourceArena;
    Scope<RenderGraph::ResourceLruPool> m_ResourceLruPool;
};

class RenderGraphTests : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        auto* testLayer = new TestLayer();
        m_Layers.push_back(testLayer);
        window.PushLayer(testLayer);
    }
    virtual void OnShutdown() override
    {
        for (auto* layer : m_Layers)
        {
            delete layer;
        }
    }
    virtual void OnFrameBegin() override
    {
    }
    virtual const char* GetName() const override
    {
        return "RenderGraphTests";
    }

private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow;
};
DEFINE_APPLICATION(RenderGraphTests);