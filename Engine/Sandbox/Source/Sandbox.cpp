#include <Debug/Log.h>
#include <Entry/Application.h>
#include <Render/RHI.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <Window/Layer.h>
#include <Window/Window.h>
#include <SDL3/SDL.h>

using namespace Aether;

namespace
{
constexpr int TitleBarHeight = 38;
constexpr int ResizeBorderWidth = 6;
constexpr int TitleBarButtonWidth = 46;

SDL_HitTestResult SDLCALL SandboxWindowHitTest(SDL_Window* window, const SDL_Point* point, void*)
{
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) == 0)
    {
        const bool left = point->x < ResizeBorderWidth;
        const bool right = point->x >= width - ResizeBorderWidth;
        const bool top = point->y < ResizeBorderWidth;
        const bool bottom = point->y >= height - ResizeBorderWidth;
        if (top && left) return SDL_HITTEST_RESIZE_TOPLEFT;
        if (top && right) return SDL_HITTEST_RESIZE_TOPRIGHT;
        if (bottom && left) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
        if (bottom && right) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
        if (left) return SDL_HITTEST_RESIZE_LEFT;
        if (right) return SDL_HITTEST_RESIZE_RIGHT;
        if (top) return SDL_HITTEST_RESIZE_TOP;
        if (bottom) return SDL_HITTEST_RESIZE_BOTTOM;
    }
    if (point->y < TitleBarHeight && point->x < width - 3 * TitleBarButtonWidth)
        return SDL_HITTEST_DRAGGABLE;
    return SDL_HITTEST_NORMAL;
}

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
        SDL_SetWindowMinimumSize(window->GetHandle(), 400, 300);
        if (!SDL_SetWindowHitTest(window->GetHandle(), SandboxWindowHitTest, nullptr))
            LogE("Failed to set Sandbox window hit test: {}", SDL_GetError());

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
    void OnImGuiUpdate() override
    {
        ImGui::Begin("CircleLayer");
        ImGui::Text("This is a simple example of using RenderGraph to render a green circle.");
        ImGui::End();
        DrawTitleBar();
    }

private:
    void DrawTitleBar()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, static_cast<float>(TitleBarHeight)));
        ImGui::SetNextWindowViewport(viewport->ID);

        constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
                                           ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.13f, 0.15f, 0.19f, 1.0f));
        ImGui::Begin("##SandboxTitleBar", nullptr, flags);

        ImGui::GetWindowDrawList()->AddText(
            ImVec2(viewport->Pos.x + 12.0f, viewport->Pos.y + 11.0f),
            IM_COL32(240, 242, 246, 255), "Aether Sandbox");

        ImGui::SetCursorScreenPos(ImVec2(viewport->Pos.x + viewport->Size.x - 3.0f * TitleBarButtonWidth,
                                         viewport->Pos.y));
        if (ImGui::Button("-##Minimize", ImVec2(TitleBarButtonWidth, TitleBarHeight)))
            SDL_MinimizeWindow(m_Window->GetHandle());
        ImGui::SameLine();

        const bool maximized = (SDL_GetWindowFlags(m_Window->GetHandle()) & SDL_WINDOW_MAXIMIZED) != 0;
        if (ImGui::Button(maximized ? "o##Maximize" : "[]##Maximize",
                          ImVec2(TitleBarButtonWidth, TitleBarHeight)))
        {
            if (maximized)
                SDL_RestoreWindow(m_Window->GetHandle());
            else
                SDL_MaximizeWindow(m_Window->GetHandle());
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.82f, 0.18f, 0.20f, 1.0f));
        if (ImGui::Button("X##Close", ImVec2(TitleBarButtonWidth, TitleBarHeight)))
        {
            SDL_Event event{};
            event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
            event.window.windowID = SDL_GetWindowID(m_Window->GetHandle());
            SDL_PushEvent(&event);
        }
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(5);
    }

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
        param.noDecorate = true;
        param.imGuiEnableClear = false;
        return param;
    }

private:
    CircleLayer* m_CircleLayer = nullptr;
};

DEFINE_APPLICATION(Sandbox);
