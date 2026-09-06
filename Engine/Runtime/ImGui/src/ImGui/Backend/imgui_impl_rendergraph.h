#pragma once
#include <ImGui/Core/imgui.h>
#include <Core/Math/Def.h>
#include <Render/PixelFormat.h>

namespace Aether
{
class PendingUploadList;
namespace rhi
{
class Texture2D;
class TextureView;
class Sampler;
class CommandList;
class Pipeline;
}
namespace RenderGraph
{
class RenderGraph;
template <typename T> struct AccessId;
}
}

// Renderer for Aether's RenderGraph. Pair with a platform backend (e.g. SDL3).
// Supports dynamic font textures, user textures, clipping and large meshes.
// Platform viewports are not implemented.
struct ImGui_ImplRenderGraph_InitInfo
{
    Aether::PixelFormat ColorFormat = Aether::PixelFormat::RGBA8888;
};

IMGUI_IMPL_API bool ImGui_ImplRenderGraph_Init(const ImGui_ImplRenderGraph_InitInfo& info = {});
// The caller must wait for GPU work before shutdown or removing user textures.
IMGUI_IMPL_API void ImGui_ImplRenderGraph_Shutdown();
IMGUI_IMPL_API void ImGui_ImplRenderGraph_NewFrame();
// Call once per rendered frame after waiting for its frame-slot fence, before
// PendingUploadList::RecordCommand(). Copies atlas updates into owned staging
// buffers and retires old textures after MaxFramesInFlight submitted frames.
IMGUI_IMPL_API void ImGui_ImplRenderGraph_UpdateTextures(
    ImDrawData* drawData, Aether::PendingUploadList& uploads);

// Textures must remain alive and in ShaderReadOnly layout between graph executions.
// IDs belong to this backend; Vulkan backend descriptor IDs are not interchangeable.
IMGUI_IMPL_API ImTextureID ImGui_ImplRenderGraph_AddTexture(
    Aether::rhi::Texture2D& texture, Aether::rhi::TextureView& view, Aether::rhi::Sampler& sampler);
IMGUI_IMPL_API void ImGui_ImplRenderGraph_RemoveTexture(ImTextureID texture);

// Adds a draw task to an uncompiled graph. Execute before the next ImGui::NewFrame(),
// and keep the graph/arena/pool alive until the submitted frame has completed.
IMGUI_IMPL_API void ImGui_ImplRenderGraph_RenderDrawData(
    ImDrawData* drawData, Aether::RenderGraph::RenderGraph& graph,
    Aether::RenderGraph::AccessId<Aether::rhi::Texture2D> target,
    bool clear = false, const Aether::Vec4f& clearColor = Aether::Vec4f(0, 0, 0, 0));

struct ImGui_ImplRenderGraph_RenderState
{
    Aether::rhi::CommandList* CommandList;
    Aether::rhi::Pipeline* Pipeline;
};
