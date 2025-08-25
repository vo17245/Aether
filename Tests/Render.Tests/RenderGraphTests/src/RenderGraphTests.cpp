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
#include <IO/Image.h>
#include <Debug/Log.h>
using namespace Aether;
static const char* vertexShaderCode = R"(
#version 450
layout(location = 0) in vec2 a_Position;
layout(location = 2) in vec2 a_TexCoord;
layout(location=0) out vec2 v_TexCoord;
void main()
{
    v_TexCoord=a_TexCoord;
    gl_Position = vec4(a_Position,0.0,  1.0);
}
)";
static const char* fragmentShaderCode = R"(
#version 450
layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 0) uniform sampler2D u_Texture;
layout(location = 0) in vec2 v_TexCoord;

void main()
{
    outColor = texture(u_Texture, v_TexCoord);
}
)";

class TestLayer : public Layer
{
public:
    ~TestLayer()
    {
    }

    virtual void OnAttach(Window* window) override
    {
        m_Window = window;
        // create command buffer pool
        m_CommandPool = vk::GraphicsCommandPool::CreateScope();
        //// create cpu grid
        auto quad = Geometry::CreateQuad();
        //// create gpu grid
        m_Mesh = VkMesh::CreateScope(*m_CommandPool, quad);
        // compile shader in cpu
        auto vsBin = Shader::Compiler::GLSL2SPIRV(&vertexShaderCode, 1, ShaderStageType::Vertex);
        if (!vsBin)
        {
            Debug::Log::Error("compile vertex shader failed:{}", vsBin.error());
            assert(vsBin);
        }
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
            auto renderPass = DeviceRenderPass::Create(desc);
            auto descriptorSet=window->GetCurrentDescriptorPool().CreateSet(0, 0, 1);
            auto& descriptorSetVk = descriptorSet.GetVk();
            // create pipeline
            vk::PipelineLayout::Builder layoutBuilder;
            layoutBuilder.AddDescriptorSetLayouts(descriptorSetVk.layouts);
            auto pipelineLayout = layoutBuilder.Build();
            vk::GraphicsPipeline::Builder pipelineBuilder(renderPass.GetVk(), *pipelineLayout);
            pipelineBuilder.AddFragmentStage(*m_FragmentShader, "main");
            pipelineBuilder.AddVertexStage(*m_VertexShader, "main");
            pipelineBuilder.PushVertexBufferLayouts(m_Mesh->GetVertexBufferLayouts());
            m_Pipeline = CreateScope<DevicePipeline>(pipelineBuilder.Build().value());
            m_PipelineLayout = CreateScope<DevicePipelineLayout>(std::move(pipelineLayout.value()));
        }
        { // load texture
            // load
            auto imageOpt = Image::LoadFromFile("Assets/bundle/Images/logo.png");
            assert(imageOpt);
            auto& image = imageOpt.value();
            // allocate device texture
            {
                auto texture = DeviceTexture::Create(image.GetWidth(), image.GetHeight(), PixelFormat::RGBA8888,
                                                     PackFlags(DeviceImageUsage::Sample, DeviceImageUsage::Upload),
                                                     DeviceImageLayout::Undefined);
                m_Texture = CreateScope<DeviceTexture>(std::move(texture));
            }

            // upload
            auto stagingBuffer = DeviceBuffer::CreateForStaging(image.GetWidth() * image.GetHeight() * 4);
            stagingBuffer.SetData(std::span<const uint8_t>(image.GetData(), image.GetDataSize()));
            m_Texture->SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::TransferDst);
            m_Texture->CopyBuffer(stagingBuffer);
            m_Texture->SyncTransitionLayout(DeviceImageLayout::TransferDst, DeviceImageLayout::Texture);
            // sampler
            m_Sampler = CreateScope<DeviceSampler>(DeviceSampler::CreateDefault());
        }
    }
    virtual void RegisterRenderPasses(RenderGraph::RenderGraph& renderGraph) override
    {
        struct TaskData
        {
        };

        Vec2i size = m_Window->GetSize();

        auto taskData = renderGraph.AddRenderTask<TaskData>(
            "test render graph1",
            [&](RenderGraph::RenderTaskBuilder& builder, TaskData& data) {
                // render pass
                auto imageView = builder.Create<DeviceImageView>(
                    "final image view",
                    RenderGraph::ImageViewDesc{.texture = m_Window->GetFinalImageAccessId(), .desc = {}});
                RenderGraph::RenderPassDesc renderPassDesc;
                renderPassDesc.colorAttachmentCount = 1;
                renderPassDesc.colorAttachment[0].imageView = imageView;
                renderPassDesc.colorAttachment[0].loadOp = DeviceAttachmentLoadOp::Clear;
                renderPassDesc.colorAttachment[0].storeOp = DeviceAttachmentStoreOp::Store;
                renderPassDesc.width = size.x();
                renderPassDesc.height = size.y();
                builder.SetRenderPassDesc(renderPassDesc);
            },
            [size, this](DeviceCommandBuffer& _commandBuffer, RenderGraph::ResourceAccessor& accessor, TaskData& data) {
                // create descriptor set
                auto descriptorSet = m_Window->GetCurrentDescriptorPool().CreateSet(0, 0, 1);
                descriptorSet.UpdateSampler(*m_Sampler, m_Texture->GetOrCreateDefaultImageView());
                // record
                _commandBuffer.BindDescriptorSet(descriptorSet, *m_PipelineLayout);
                auto& commandBuffer = _commandBuffer.GetVk();
                _commandBuffer.SetViewport(0, 0, size.x(), size.y());
                _commandBuffer.SetScissor(0, 0, size.x(), size.y());
                _commandBuffer.BindPipeline(*m_Pipeline);

                Render::Utils::DrawMesh(commandBuffer, *m_Mesh);
            });
    }
    virtual bool NeedRebuildRenderGraph() override
    {
        return m_NeedRebuild;
    }

private:
    Scope<DevicePipeline> m_Pipeline;
    Scope<DevicePipelineLayout> m_PipelineLayout;
    Scope<VkMesh> m_Mesh;
    Scope<vk::GraphicsCommandPool> m_CommandPool;
    Scope<vk::DescriptorSet> m_DescriptorSet;
    Scope<vk::ShaderModule> m_VertexShader;
    Scope<vk::ShaderModule> m_FragmentShader;
    Window* m_Window = nullptr;
    Scope<DeviceTexture> m_Texture;
    Scope<DeviceSampler> m_Sampler;
    bool m_NeedRebuild;
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