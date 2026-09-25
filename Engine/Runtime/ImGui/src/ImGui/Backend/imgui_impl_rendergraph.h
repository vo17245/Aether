#pragma once
#include <ImGui/Core/imgui.h>
#include <Core/Math/Def.h>
#include <Render/PixelFormat.h>
#include <ImGui/Compat/ImGuiRenderPacket.h>
#include <Core/DisplaySurfaceToken.h>
#include <Render/RenderGraph/Resource/ResourceId.h>
#include <functional>
#include <memory>
#include <span>

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

struct ImGui_ImplRenderGraph_Backend;
using ImGui_ImplRenderGraph_Callback = std::function<void(
    Aether::rhi::CommandList&, std::span<const std::byte>)>;
struct ImGui_ImplRenderGraph_DisplaySurfaceBinding
{
    Aether::rhi::Texture2D* texture = nullptr;
    Aether::rhi::TextureView* view = nullptr;
    Aether::rhi::Sampler* sampler = nullptr;
    Aether::RenderGraph::ResourceId<Aether::rhi::Texture2D> resourceId{};
    std::shared_ptr<const void> lease;
};
using ImGui_ImplRenderGraph_DisplaySurfaceResolver = std::function<std::optional<
    ImGui_ImplRenderGraph_DisplaySurfaceBinding>(Aether::DisplaySurfaceToken,
        const std::shared_ptr<const void>&, std::uint32_t)>;

IMGUI_IMPL_API bool ImGui_ImplRenderGraph_Init(const ImGui_ImplRenderGraph_InitInfo& info = {});
IMGUI_IMPL_API ImGui_ImplRenderGraph_Backend* ImGui_ImplRenderGraph_CreateBackend(
    const ImGui_ImplRenderGraph_InitInfo& info = {});
IMGUI_IMPL_API void ImGui_ImplRenderGraph_DestroyBackend(ImGui_ImplRenderGraph_Backend* backend);
// The caller must wait for GPU work before shutdown or removing user textures.
IMGUI_IMPL_API void ImGui_ImplRenderGraph_Shutdown();
IMGUI_IMPL_API void ImGui_ImplRenderGraph_NewFrame();
// Frontend-only compatibility accessor. Render work must receive this pointer
// explicitly and must not call ImGui::GetIO/GetPlatformIO.
IMGUI_IMPL_API ImGui_ImplRenderGraph_Backend* ImGui_ImplRenderGraph_GetBackend();
// Call once per rendered frame after waiting for its frame-slot fence, before
// PendingUploadList::RecordCommand(). Copies atlas updates into owned staging
// buffers. PendingUploadList retires staging after the corresponding slot fence.
IMGUI_IMPL_API void ImGui_ImplRenderGraph_UpdateTextures(
    ImDrawData* drawData, Aether::PendingUploadList& uploads);

IMGUI_IMPL_API void ImGui_ImplRenderGraph_ApplyTextureOperations(
    ImGui_ImplRenderGraph_Backend& backend,
    std::span<const Aether::ImGuiCompat::ImGuiTextureOperation> operations,
    Aether::PendingUploadList& uploads);
IMGUI_IMPL_API void ImGui_ImplRenderGraph_OnFrameSlotCompleted(
    ImGui_ImplRenderGraph_Backend& backend, std::uint32_t frameSlot);
IMGUI_IMPL_API void ImGui_ImplRenderGraph_RegisterCallback(
    ImGui_ImplRenderGraph_Backend& backend, Aether::ImGuiCompat::RenderCallbackId id,
    ImGui_ImplRenderGraph_Callback callback);
IMGUI_IMPL_API void ImGui_ImplRenderGraph_UnregisterCallback(
    ImGui_ImplRenderGraph_Backend& backend, Aether::ImGuiCompat::RenderCallbackId id);
IMGUI_IMPL_API void ImGui_ImplRenderGraph_SetDisplaySurfaceResolver(
    ImGui_ImplRenderGraph_Backend& backend, ImGui_ImplRenderGraph_DisplaySurfaceResolver resolver);

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

IMGUI_IMPL_API void ImGui_ImplRenderGraph_RenderPacket(
    ImGui_ImplRenderGraph_Backend& backend,
    std::shared_ptr<const Aether::ImGuiCompat::ImGuiRenderPacket> packet,
    Aether::RenderGraph::RenderGraph& graph,
    Aether::RenderGraph::AccessId<Aether::rhi::Texture2D> target,
    bool clear = false, const Aether::Vec4f& clearColor = Aether::Vec4f(0, 0, 0, 0),
    std::uint32_t frameSlot = 0);

struct ImGui_ImplRenderGraph_RenderState
{
    Aether::rhi::CommandList* CommandList;
    Aether::rhi::Pipeline* Pipeline;
};
