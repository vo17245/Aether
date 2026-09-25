#include "ImGuiRenderPacket.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace Aether::ImGuiCompat
{
namespace
{
template <typename T>
bool FitsU32(T value)
{
    return value <= static_cast<T>(std::numeric_limits<std::uint32_t>::max());
}

void CopyRegion(const ImTextureData& texture, const ImTextureRect& sourceRect, ImGuiTextureRegion& destination)
{
    destination.x = sourceRect.x;
    destination.y = sourceRect.y;
    destination.width = sourceRect.w;
    destination.height = sourceRect.h;
    destination.rowBytes = destination.width * static_cast<std::uint32_t>(texture.BytesPerPixel);
    destination.pixels.resize(static_cast<std::size_t>(destination.rowBytes) * destination.height);
    const auto* source = reinterpret_cast<const std::byte*>(texture.Pixels);
    for (std::uint32_t row = 0; row < destination.height; ++row)
    {
        const auto sourceOffset =
            (static_cast<std::size_t>(destination.y + row) * texture.Width + destination.x) * texture.BytesPerPixel;
        std::memcpy(destination.pixels.data() + static_cast<std::size_t>(row) * destination.rowBytes,
                    source + sourceOffset, destination.rowBytes);
    }
}

bool IsValidRegion(const ImTextureData& texture, const ImTextureRect& region)
{
    return region.w > 0 && region.h > 0 && region.x <= texture.Width && region.y <= texture.Height &&
           static_cast<std::uint32_t>(region.x) + region.w <= static_cast<std::uint32_t>(texture.Width) &&
           static_cast<std::uint32_t>(region.y) + region.h <= static_cast<std::uint32_t>(texture.Height);
}
} // namespace

std::size_t ImGuiRenderPacket::PayloadBytes() const noexcept
{
    std::size_t result = vertices.size() * sizeof(ImGuiPacketVertex) + indices.size() * sizeof(std::uint32_t) +
                         commands.size() * sizeof(ImGuiPacketCommand);
    for (const auto& command : commands) result += command.callbackPayload.size();
    return result;
}

std::size_t ImGuiTextureOperation::PayloadBytes() const noexcept
{
    std::size_t result = fullPixels.size();
    for (const auto& region : regions) result += region.pixels.size();
    return result;
}

void ImGuiRenderPacketExtractor::RegisterCallback(ImDrawCallback callback, RenderCallbackId id)
{
    if (callback && callback != ImDrawCallback_ResetRenderState && id != 0) m_Callbacks[callback] = id;
}

void ImGuiRenderPacketExtractor::UnregisterCallback(ImDrawCallback callback)
{
    m_Callbacks.erase(callback);
}

TextureId ImGuiRenderPacketExtractor::AllocateTextureId()
{
    if (m_NextTexture == 0) ++m_NextTexture;
    return m_NextTexture++;
}

TextureId ImGuiRenderPacketExtractor::ResolveUserTexture(ImTextureID texture)
{
    if (texture == ImTextureID_Invalid) return 0;
    auto [iterator, inserted] = m_UserTextures.try_emplace(texture, static_cast<TextureId>(texture));
    (void)inserted;
    return iterator->second;
}

TextureId ImGuiRenderPacketExtractor::ResolveTextureData(ImTextureData* texture)
{
    auto [iterator, inserted] = m_TextureData.try_emplace(texture, 0);
    if (inserted)
        iterator->second = texture && texture->GetTexID() != ImTextureID_Invalid
                               ? static_cast<TextureId>(texture->GetTexID())
                               : AllocateTextureId();
    return iterator->second;
}

std::expected<ImGuiRenderPacketExtractor::Extraction, ImGuiExtractError>
ImGuiRenderPacketExtractor::Extract(const ImDrawData& drawData)
{
    if (!drawData.Valid || drawData.CmdListsCount < 0 || drawData.CmdListsCount != drawData.CmdLists.Size ||
        drawData.TotalVtxCount < 0 || drawData.TotalIdxCount < 0)
        return std::unexpected(ImGuiExtractError{ImGuiExtractErrorCode::InvalidDrawData, "invalid ImDrawData"});

    Extraction result;
    result.packet.displayPos[0] = drawData.DisplayPos.x;
    result.packet.displayPos[1] = drawData.DisplayPos.y;
    result.packet.displaySize[0] = drawData.DisplaySize.x;
    result.packet.displaySize[1] = drawData.DisplaySize.y;
    result.packet.framebufferScale[0] = drawData.FramebufferScale.x;
    result.packet.framebufferScale[1] = drawData.FramebufferScale.y;
    result.packet.vertices.reserve(static_cast<std::size_t>(drawData.TotalVtxCount));
    result.packet.indices.reserve(static_cast<std::size_t>(drawData.TotalIdxCount));

    if (drawData.Textures)
    {
        for (ImTextureData* texture : *drawData.Textures)
        {
            if (!texture) continue;
            const TextureId logicalId = ResolveTextureData(texture);
            if (texture->Status == ImTextureStatus_WantDestroy)
            {
                result.textureOperations.push_back(
                    {.type = ImGuiTextureOperationType::Destroy, .texture = logicalId, .format = texture->Format,
                     .width = static_cast<std::uint32_t>(std::max(texture->Width, 0)),
                     .height = static_cast<std::uint32_t>(std::max(texture->Height, 0))});
                result.acknowledgements.push_back({texture, logicalId, ImGuiTextureOperationType::Destroy});
                continue;
            }
            if (texture->Status != ImTextureStatus_WantCreate && texture->Status != ImTextureStatus_WantUpdates)
                continue;
            if (texture->Format != ImTextureFormat_RGBA32 && texture->Format != ImTextureFormat_Alpha8)
                return std::unexpected(
                    ImGuiExtractError{ImGuiExtractErrorCode::InvalidTexture, "unsupported ImGui texture format"});
            const int expectedBytesPerPixel = texture->Format == ImTextureFormat_RGBA32 ? 4 : 1;
            if (!texture->Pixels || texture->Width <= 0 || texture->Height <= 0 ||
                texture->Width > std::numeric_limits<unsigned short>::max() ||
                texture->Height > std::numeric_limits<unsigned short>::max() ||
                texture->BytesPerPixel != expectedBytesPerPixel)
                return std::unexpected(
                    ImGuiExtractError{ImGuiExtractErrorCode::InvalidTexture, "invalid ImGui texture update"});

            ImGuiTextureOperation operation{
                .type = texture->Status == ImTextureStatus_WantCreate ? ImGuiTextureOperationType::Create
                                                                      : ImGuiTextureOperationType::Update,
                .texture = logicalId,
                .format = texture->Format,
                .width = static_cast<std::uint32_t>(texture->Width),
                .height = static_cast<std::uint32_t>(texture->Height),
            };
            if (operation.type == ImGuiTextureOperationType::Create)
            {
                ImTextureRect full{0, 0, static_cast<unsigned short>(texture->Width),
                                   static_cast<unsigned short>(texture->Height)};
                operation.regions.emplace_back();
                CopyRegion(*texture, full, operation.regions.back());
            }
            else
            {
                if (!texture->Updates.empty())
                {
                    for (const ImTextureRect& update : texture->Updates)
                    {
                        if (!IsValidRegion(*texture, update))
                            return std::unexpected(ImGuiExtractError{ImGuiExtractErrorCode::InvalidTexture,
                                                                     "ImGui texture update is out of bounds"});
                        operation.regions.emplace_back();
                        CopyRegion(*texture, update, operation.regions.back());
                    }
                }
                else if (texture->UpdateRect.w && texture->UpdateRect.h)
                {
                    if (!IsValidRegion(*texture, texture->UpdateRect))
                        return std::unexpected(ImGuiExtractError{ImGuiExtractErrorCode::InvalidTexture,
                                                                 "ImGui texture update is out of bounds"});
                    operation.regions.emplace_back();
                    CopyRegion(*texture, texture->UpdateRect, operation.regions.back());
                }
                operation.fullRowBytes = operation.width * static_cast<std::uint32_t>(texture->BytesPerPixel);
                operation.fullPixels.resize(static_cast<std::size_t>(operation.fullRowBytes) * operation.height);
                std::memcpy(operation.fullPixels.data(), texture->Pixels, operation.fullPixels.size());
            }
            result.textureOperations.push_back(std::move(operation));
            result.acknowledgements.push_back(
                {texture, logicalId, texture->Status == ImTextureStatus_WantCreate
                                         ? ImGuiTextureOperationType::Create
                                         : ImGuiTextureOperationType::Update});
        }
    }

    std::uint32_t baseVertex = 0;
    std::uint32_t baseIndex = 0;
    for (const ImDrawList* list : drawData.CmdLists)
    {
        if (!list || !FitsU32(result.packet.vertices.size() + list->VtxBuffer.size()) ||
            !FitsU32(result.packet.indices.size() + list->IdxBuffer.size()))
            return std::unexpected(
                ImGuiExtractError{ImGuiExtractErrorCode::InvalidDrawData, "ImGui draw buffers exceed packet limits"});
        for (const ImDrawVert& vertex : list->VtxBuffer)
            result.packet.vertices.push_back({{vertex.pos.x, vertex.pos.y}, {vertex.uv.x, vertex.uv.y}, vertex.col});
        for (const ImDrawIdx index : list->IdxBuffer) result.packet.indices.push_back(index);

        for (const ImDrawCmd& source : list->CmdBuffer)
        {
            if (source.IdxOffset > static_cast<unsigned int>(list->IdxBuffer.Size) ||
                source.ElemCount > static_cast<unsigned int>(list->IdxBuffer.Size) - source.IdxOffset ||
                source.VtxOffset > static_cast<unsigned int>(list->VtxBuffer.Size))
                return std::unexpected(
                    ImGuiExtractError{ImGuiExtractErrorCode::InvalidDrawData, "draw command offsets are out of bounds"});
            ImGuiPacketCommand command;
            std::copy_n(&source.ClipRect.x, 4, command.clipRect);
            command.elementCount = source.ElemCount;
            command.indexOffset = baseIndex + source.IdxOffset;
            command.vertexOffset = baseVertex + source.VtxOffset;
            if (source.UserCallback)
            {
                if (source.UserCallback == ImDrawCallback_ResetRenderState)
                {
                    command.type = ImGuiPacketCommandType::ResetRenderState;
                }
                else
                {
                    const auto callback = m_Callbacks.find(source.UserCallback);
                    if (callback == m_Callbacks.end() ||
                        (source.UserCallbackDataSize == 0 && source.UserCallbackData != nullptr) ||
                        (source.UserCallbackDataSize > 0 && source.UserCallbackData == nullptr))
                        return std::unexpected(ImGuiExtractError{ImGuiExtractErrorCode::UnsupportedCallback,
                                                                 "ImGui callback has no registered owned protocol"});
                    command.type = ImGuiPacketCommandType::Callback;
                    command.callback = callback->second;
                    if (source.UserCallbackDataSize > 0)
                    {
                        command.callbackPayload.resize(static_cast<std::size_t>(source.UserCallbackDataSize));
                        std::memcpy(command.callbackPayload.data(), source.UserCallbackData,
                                    command.callbackPayload.size());
                    }
                }
            }
            else
            {
                command.type = ImGuiPacketCommandType::Draw;
                if (source.TexRef._TexData)
                    command.texture = ResolveTextureData(source.TexRef._TexData);
                else if (source.TexRef._TexID != ImTextureID_Invalid)
                {
                    const auto surface = m_DisplaySurfaces.find(source.TexRef._TexID);
                    if (surface != m_DisplaySurfaces.end())
                    {
                        command.displaySurface = surface->second.token;
                        command.displaySurfacePin = surface->second.pin;
                    }
                    else command.texture = ResolveUserTexture(source.TexRef._TexID);
                }
                else
                    return std::unexpected(
                        ImGuiExtractError{ImGuiExtractErrorCode::InvalidTexture, "draw command has no texture"});
            }
            result.packet.commands.push_back(std::move(command));
        }
        baseVertex += static_cast<std::uint32_t>(list->VtxBuffer.size());
        baseIndex += static_cast<std::uint32_t>(list->IdxBuffer.size());
    }
    if (result.packet.vertices.size() != static_cast<std::size_t>(drawData.TotalVtxCount) ||
        result.packet.indices.size() != static_cast<std::size_t>(drawData.TotalIdxCount))
        return std::unexpected(
            ImGuiExtractError{ImGuiExtractErrorCode::InvalidDrawData, "ImDrawData totals do not match its lists"});
    return result;
}

bool ImGuiRenderPacketExtractor::CommitAccepted(Extraction& extraction)
{
    if (extraction.committed) return false;
    for (const auto& acknowledgement : extraction.acknowledgements)
    {
        ImTextureData* texture = acknowledgement.texture;
        if (!texture) continue;
        if (acknowledgement.type == ImGuiTextureOperationType::Destroy)
        {
            texture->SetTexID(ImTextureID_Invalid);
            texture->BackendUserData = nullptr;
            texture->SetStatus(ImTextureStatus_Destroyed);
            m_TextureData.erase(texture);
        }
        else
        {
            texture->SetTexID(static_cast<ImTextureID>(acknowledgement.logicalId));
            texture->SetStatus(ImTextureStatus_OK);
        }
    }
    extraction.committed = true;
    return true;
}

ImTextureID ImGuiRenderPacketExtractor::RegisterDisplaySurface(
    DisplaySurfaceToken token, std::shared_ptr<const void> lifetimePin)
{
    if (!token.IsValid() || !lifetimePin) return ImTextureID_Invalid;
    while (m_DisplaySurfaces.contains(m_NextDisplaySurface) || m_UserTextures.contains(m_NextDisplaySurface))
    {
        if (m_NextDisplaySurface == std::numeric_limits<ImTextureID>::max()) return ImTextureID_Invalid;
        ++m_NextDisplaySurface;
    }
    const auto handle = m_NextDisplaySurface;
    m_DisplaySurfaces.emplace(handle, DisplaySurfaceRegistration{token, std::move(lifetimePin)});
    if (m_NextDisplaySurface != std::numeric_limits<ImTextureID>::max()) ++m_NextDisplaySurface;
    return handle;
}

bool ImGuiRenderPacketExtractor::UnregisterDisplaySurface(ImTextureID textureId) noexcept
{
    return m_DisplaySurfaces.erase(textureId) != 0;
}
} // namespace Aether::ImGuiCompat
