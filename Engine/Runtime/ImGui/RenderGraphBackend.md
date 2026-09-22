# RenderGraph backend

`ImGui/Backend/imgui_impl_rendergraph.h` is the renderer interface. SDL3 remains
the main-thread platform backend. `ImGuiApi::InitFrontend` creates only the
ImGui context and SDL backend. Window creates the explicit RenderGraph backend
on RenderThread and destroys it there before `ShutdownFrontend`.

Window records scene rendering first, then an ImGui RenderGraph targeting its
current final texture, then copies the result to the swapchain. Each UI graph,
arena and pool lives until that frame slot's fence completes. UI clearing keeps
the `WindowCreateParam::imGuiEnableClear` behavior; otherwise UI loads the scene.

## Upload ordering

The main thread extracts an owning `ImGuiRenderPacket` after `ImGui::Render()`.
Dynamic texture operations and the draw packet are accepted atomically by the
bounded RenderThread queue. ImGui texture status is acknowledged only after
that acceptance. On RenderThread, after waiting for the current frame slot:

1. Call `PendingUploadList::OnFrameSlotCompleted(slot)` after that slot fence signals.
2. Apply reliable texture operations to the explicit backend.
3. Begin recording the frame command list, and call `uploads.RecordCommand(commands, slot)`.
4. Add `ImGui_ImplRenderGraph_RenderPacket()` to an uncompiled graph, compile
   it and execute it. The main thread may already be building its next frame.

Font creation uploads the whole atlas. Font updates upload `UpdateRect` with
tightly packed rows through `PendingUploadList::UploadTexture`. The upload list
owns the staging bytes and records Undefined/ShaderReadOnly -> TransferDst ->
ShaderReadOnly transitions. Minimized windows use an upload-only graphics
submission, so reliable updates continue to progress. Retired font textures
and descriptors are released only after every frame-slot fence that could
reference them has completed. This path requires no per-frame queue-idle wait
or synchronous texture copy.

## User textures

The old compatibility backend can register with
`ImGui_ImplRenderGraph_AddTexture(texture, view, sampler)` and pass
the returned ID through `ImTextureRef` to `ImGui::Image`. Texture IDs from
`ImGui_ImplVulkan_AddTexture` are incompatible. Keep all three resources alive
until their last submission completes, then call `ImGui_ImplRenderGraph_RemoveTexture`.
That API still requires the legacy synchronous backend and is outside the
default RenderThread-supported Sandbox scope. New asynchronous callers require
a renderer-owned resource-handle protocol.
Externally registered textures must be sampleable and in ShaderReadOnly layout
between graph executions; the backend imports their read dependencies into the
UI graph. Do not sample the same image that the UI pass writes.

The backend supports DisplayPos/FramebufferScale, clipping, 16/32-bit indices,
large meshes with VtxOffset, reset-state callbacks and dynamic font textures.
It does not advertise platform viewport support. The current RHI's Vulkan
pipeline builder and bindings supply operations not yet exposed by CommandList.

## GPU integration check

`ImGuiPacket.Tests` covers owning draw data, dynamic texture acceptance and
callback payload extraction without a GPU. The legacy `ImGui.RenderGraph.Tests`
still uses main-thread RHI user textures/readback and is intentionally excluded
until it is migrated to renderer-owned handles and readback commands.
