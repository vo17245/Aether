#include <Debug/Log.h>
#include <Entry/Application.h>
#include <Render/RHI.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <Window/Layer.h>
#include <Window/Window.h>

using namespace Aether;

namespace
{
constexpr const char* VertexShaderCode = R"(
#version 450

layout(location = 0) out vec2 v_Position;

void main()
{
    const vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    v_Position = positions[gl_VertexIndex];
    gl_Position = vec4(v_Position, 0.0, 1.0);
}
)";

constexpr const char* PixelShaderCode = R"(
#version 450

layout(location = 0) in vec2 v_Position;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 pixelsFromCenter = v_Position / fwidth(v_Position);
    vec2 viewportSize = 2.0 / fwidth(v_Position);
    float radius = 0.2 * min(viewportSize.x, viewportSize.y);
    float edge = 1.0 - smoothstep(radius - 1.0, radius + 1.0, length(pixelsFromCenter));
    outColor = vec4(0.0, edge, 0.0, 1.0);
}
)";

class CircleLayer final : public Layer
{
public:
    void OnAttach(Window* window) override
    {
        m_Window = window;

        auto vertexShader = rhi::VertexShader::Create(
            ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, VertexShaderCode));
        if (!vertexShader)
        {
            LogE("Failed to create circle vertex shader: {}", vertexShader.error());
            assert(false && "failed to create circle vertex shader");
            return;
        }
        m_VertexShader = std::move(*vertexShader);

        auto pixelShader = rhi::PixelShader::Create(
            ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, PixelShaderCode));
        if (!pixelShader)
        {
            LogE("Failed to create circle pixel shader: {}", pixelShader.error());
            assert(false && "failed to create circle pixel shader");
            return;
        }
        m_PixelShader = std::move(*pixelShader);

        rhi::PipelineDesc pipelineDesc;
        pipelineDesc.colorAttachmentFormats = {PixelFormat::RGBA8888};
        pipelineDesc.vertexShader = &m_VertexShader;
        pipelineDesc.pixelShader = &m_PixelShader;
        m_Pipeline = rhi::Pipeline::Create(pipelineDesc);
        assert(m_Pipeline && "failed to create circle pipeline");
    }

    void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph) override
    {
        struct TaskData
        {
            CircleLayer* layer = nullptr;
            uint32_t width = 0;
            uint32_t height = 0;
        };

        const Vec2i size = m_Window->GetSize();
        renderGraph.AddRenderTask<TaskData>(
            "Sandbox.GreenCircle",
            [&](RenderGraph::RenderTaskBuilder& builder, TaskData& data) {
                auto targetView = builder.Create<rhi::TextureView>(
                    "Sandbox.GreenCircle.TargetView",
                    RenderGraph::TextureViewDesc{
                        .texture = m_Window->GetFinalImageAccessId(),
                        .desc = {},
                    });

                RenderGraph::RenderPassDesc passDesc{};
                passDesc.colorAttachmentCount = 1;
                passDesc.colorAttachment[0] = {
                    .textureView = targetView,
                    .loadOp = rhi::AttachmentLoadOp::Clear,
                    .storeOp = rhi::AttachmentStoreOp::Store,
                };
                passDesc.clearColor[0] = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
                passDesc.width = static_cast<uint32_t>(size.x());
                passDesc.height = static_cast<uint32_t>(size.y());
                builder.SetRenderPassDesc(passDesc);

                data.layer = this;
                data.width = passDesc.width;
                data.height = passDesc.height;
            },
            [](rhi::CommandList& commandList, RenderGraph::ResourceAccessor&, TaskData& data) {
                commandList.SetViewport(0.0f, 0.0f, static_cast<float>(data.width), static_cast<float>(data.height));
                commandList.SetScissor(0.0f, 0.0f, static_cast<float>(data.width), static_cast<float>(data.height));
                commandList.BindPipeline(data.layer->m_Pipeline);
                commandList.GetVk().Draw(3);
            });
    }

private:
    Window* m_Window = nullptr;
    rhi::VertexShader m_VertexShader;
    rhi::PixelShader m_PixelShader;
    rhi::Pipeline m_Pipeline;
};
} // namespace

class Sandbox final : public Application
{
public:
    void OnInit(Window& window) override
    {
        m_CircleLayer = new CircleLayer();
        window.PushLayer(m_CircleLayer);
    }

    void OnShutdown() override
    {
        delete m_CircleLayer;
        m_CircleLayer = nullptr;
    }

    const char* GetName() const override
    {
        return "Sandbox";
    }

    WindowCreateParam MainWindowCreateParam() override
    {
        WindowCreateParam param;
        param.title = "Aether Sandbox - RenderGraph Circle";
        param.imGuiEnableClear = false;
        return param;
    }

private:
    CircleLayer* m_CircleLayer = nullptr;
};

DEFINE_APPLICATION(Sandbox);
