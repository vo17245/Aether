#include <Debug/Log.h>
#include <Entry/Application.h>
#include <Render/RHI.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <ImGui/Compat/BorderlessWindow.h>
#include <Window/Layer.h>
#include <Window/Window.h>
#include <SDL3/SDL.h>
#include <array>
#include <stdexcept>

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

struct CircleFrameData final : Render::RenderFeatureData
{
    explicit CircleFrameData(bool isVisible) : visible(isVisible) {}
    bool visible = true;
    std::size_t PayloadBytes() const noexcept override { return sizeof(*this); }
};

class CircleRenderFeature final : public Render::RenderFeature
{
public:
    void OnRenderAttach(Render::RenderFrameContext&) override
    {
        m_FrameVisible.fill(true);
        auto vertexShader = rhi::VertexShader::Create(
            ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, VertexShaderCode));
        if (!vertexShader)
            throw std::runtime_error("failed to create circle vertex shader: " + vertexShader.error());
        m_VertexShader = std::move(*vertexShader);

        auto pixelShader = rhi::PixelShader::Create(
            ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, PixelShaderCode));
        if (!pixelShader)
            throw std::runtime_error("failed to create circle pixel shader: " + pixelShader.error());
        m_PixelShader = std::move(*pixelShader);

        rhi::PipelineDesc pipelineDesc;
        pipelineDesc.colorAttachmentFormats = {PixelFormat::RGBA8888};
        pipelineDesc.vertexShader = &m_VertexShader;
        pipelineDesc.pixelShader = &m_PixelShader;
        m_Pipeline = rhi::Pipeline::Create(pipelineDesc);
        if (!m_Pipeline)
            throw std::runtime_error("failed to create circle pipeline");
    }

    void OnRenderDetach(Render::RenderFrameContext&) override
    {
        m_Pipeline = rhi::Pipeline{};
        m_PixelShader = rhi::PixelShader{};
        m_VertexShader = rhi::VertexShader{};
    }

    void BuildRenderGraph(Render::RenderGraphBuildContext& context) override
    {
        struct TaskData
        {
            std::shared_ptr<CircleRenderFeature> feature;
            uint32_t width = 0;
            uint32_t height = 0;
        };

        auto self = std::static_pointer_cast<CircleRenderFeature>(shared_from_this());
        context.graph.AddRenderTask<TaskData>(
            "Sandbox.GreenCircle",
            [&](RenderGraph::RenderTaskBuilder& builder, TaskData& data) {
                auto targetView = builder.Create<rhi::TextureView>(
                    "Sandbox.GreenCircle.TargetView",
                    RenderGraph::TextureViewDesc{
                        .texture = context.output,
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
                passDesc.width = context.width;
                passDesc.height = context.height;
                builder.SetRenderPassDesc(passDesc);

                data.feature = self;
                data.width = passDesc.width;
                data.height = passDesc.height;
            },
            [](rhi::CommandList& commandList, RenderGraph::ResourceAccessor& accessor, TaskData& data) {
                commandList.SetViewport(0.0f, 0.0f, static_cast<float>(data.width), static_cast<float>(data.height));
                commandList.SetScissor(0.0f, 0.0f, static_cast<float>(data.width), static_cast<float>(data.height));
                commandList.BindPipeline(data.feature->m_Pipeline);
                const auto slot = accessor.GetCurrentFrame() % Render::Config::MaxFramesInFlight;
                if (data.feature->m_FrameVisible[slot])
                    commandList.GetVk().Draw(3);
            });
    }

    void PrepareFrame(Render::RenderFrameContext& context, const Render::RenderFeatureData& data) override
    {
        const auto* circle = dynamic_cast<const CircleFrameData*>(&data);
        if (!circle)
            throw std::invalid_argument("CircleRenderFeature received incompatible frame data");
        m_FrameVisible[context.frameSlot % Render::Config::MaxFramesInFlight] = circle->visible;
    }

private:
    std::array<bool, Render::Config::MaxFramesInFlight> m_FrameVisible{};
    rhi::VertexShader m_VertexShader;
    rhi::PixelShader m_PixelShader;
    rhi::Pipeline m_Pipeline;
};

class CircleLayer final : public Layer
{
public:
    void OnAttach(Window* window) override
    {
        m_Window = window;
        m_RenderFeature = std::make_shared<CircleRenderFeature>();
        SDL_SetWindowMinimumSize(window->GetHandle(), 400, 300);
        if (!m_BorderlessWindow.Attach(window->GetHandle()))
            LogE("Failed to set Sandbox window hit test: {}", SDL_GetError());
    }

    void OnDetach() override
    {
        m_BorderlessWindow.Detach();
        m_RenderFeature.reset();
        m_Window = nullptr;
    }

    void CollectRenderFeatures(std::vector<std::shared_ptr<Render::RenderFeature>>& features) override
    {
        features.push_back(m_RenderFeature);
    }

    void ExtractRenderData(Render::RenderFeatureFrame& frame) override
    {
        frame.Emplace<CircleFrameData>(m_RenderFeature, m_Visible);
    }

    void OnImGuiUpdate() override
    {
        ImGui::Begin("CircleLayer");
        ImGui::Text("This is a simple example of using RenderGraph to render a green circle.");
        ImGui::Checkbox("Visible", &m_Visible);
        ImGui::End();
        m_BorderlessWindow.Draw();
    }

private:
    Window* m_Window = nullptr;
    bool m_Visible = true;
    std::shared_ptr<CircleRenderFeature> m_RenderFeature;
    ImGuiApi::BorderlessWindow m_BorderlessWindow{{.title = "Aether Sandbox"}};
};
} // namespace

class Sandbox final : public Application
{
public:
    void OnInit(Window& window) override
    {
        m_Window = &window;
        m_CircleLayer = new CircleLayer();
        window.PushLayer(m_CircleLayer);
    }

    void OnShutdown() override
    {
        if (m_Window && m_CircleLayer) m_Window->PopLayer(m_CircleLayer);
        delete m_CircleLayer;
        m_CircleLayer = nullptr;
        m_Window = nullptr;
    }

    const char* GetName() const override
    {
        return "Sandbox";
    }

    WindowCreateParam MainWindowCreateParam() override
    {
        WindowCreateParam param;
        param.title = "Aether Sandbox - RenderGraph Circle";
        param.noDecorate = true;
        param.imGuiEnableClear = false;
        return param;
    }

private:
    Window* m_Window = nullptr;
    CircleLayer* m_CircleLayer = nullptr;
};

DEFINE_APPLICATION(Sandbox);
