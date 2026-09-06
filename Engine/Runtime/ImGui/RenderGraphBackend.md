# RenderGraph backend

`ImGui/Backend/imgui_impl_rendergraph.h` is the renderer interface. SDL3 remains
the platform backend. `ImGuiApi::Init`, `NewFrame`, and `Shutdown` select it by
default. The bundled Vulkan backend remains available as reference code.

Window records scene rendering first, then an ImGui RenderGraph targeting its
current final texture, then copies the result to the swapchain. Each UI graph,
arena and pool lives until that frame slot's fence completes. UI clearing keeps
the `WindowCreateParam::imGuiEnableClear` behavior; otherwise UI loads the scene.

## Upload ordering

After waiting for the current frame slot:

1. Call `PendingUploadList::OnUpdate(false)` to age submitted staging buffers.
2. Call `ImGui_ImplRenderGraph_UpdateTextures(drawData, uploads)` once.
3. Begin recording the frame command list, and call `uploads.RecordCommand()`.
4. Add `ImGui_ImplRenderGraph_RenderDrawData()` to an uncompiled graph, compile
   it and execute it before the next `ImGui::NewFrame()`.

Font creation uploads the whole atlas. Font updates upload `UpdateRect` with
tightly packed rows through `PendingUploadList::UploadTexture`. The upload list
owns the staging bytes and records Undefined/ShaderReadOnly -> TransferDst ->
ShaderReadOnly transitions. Staging lifetime starts at command recording;
unsubmitted uploads and minimized frames do not age it. Retired font textures
and their descriptors survive `Render::Config::MaxFramesInFlight` rendered
frames. This path requires no queue-idle wait or synchronous texture copy.

## User textures

Register with `ImGui_ImplRenderGraph_AddTexture(texture, view, sampler)` and pass
the returned ID through `ImTextureRef` to `ImGui::Image`. Texture IDs from
`ImGui_ImplVulkan_AddTexture` are incompatible. Keep all three resources alive
until their last submission completes, then call `ImGui_ImplRenderGraph_RemoveTexture`.
Externally registered textures must be sampleable and in ShaderReadOnly layout
between graph executions; the backend imports their read dependencies into the
UI graph. Do not sample the same image that the UI pass writes.

The backend supports DisplayPos/FramebufferScale, clipping, 16/32-bit indices,
large meshes with VtxOffset, reset-state callbacks and dynamic font textures.
It does not advertise platform viewport support. The current RHI's Vulkan
pipeline builder and bindings supply operations not yet exposed by CommandList.

## GPU integration check

Configure with `-DAETHER_BUILD_IMGUI_TESTS=ON`, then build and run
`ImGui.RenderGraph.Tests`. It exercises dynamic fonts, a 68K+ vertex draw list,
callbacks, clipping, partial user texture uploads, resize and minimize/restore.
It reads both final-image frame slots back and checks composited RGBA pixels,
including preserved destination alpha and untouched texture regions.
