#pragma once

#include <ImGui/Core/imgui.h>
#include <Core/DisplaySurfaceToken.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <span>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Aether::ImGuiCompat
{
using TextureId = std::uint64_t;
using RenderCallbackId = std::uint64_t;

struct ImGuiPacketVertex
{
    float position[2];
    float uv[2];
    std::uint32_t color;
};

enum class ImGuiPacketCommandType
{
    Draw,
    ResetRenderState,
    Callback,
};

struct ImGuiPacketCommand
{
    ImGuiPacketCommandType type = ImGuiPacketCommandType::Draw;
    float clipRect[4]{};
    std::uint32_t elementCount = 0;
    std::uint32_t indexOffset = 0;
    std::uint32_t vertexOffset = 0;
    TextureId texture = 0;
    std::optional<DisplaySurfaceToken> displaySurface;
    std::shared_ptr<const void> displaySurfacePin;
    RenderCallbackId callback = 0;
    std::vector<std::byte> callbackPayload;
};

struct ImGuiRenderPacket
{
    float displayPos[2]{};
    float displaySize[2]{};
    float framebufferScale[2]{1.0f, 1.0f};
    std::vector<ImGuiPacketVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<ImGuiPacketCommand> commands;

    std::size_t PayloadBytes() const noexcept;
};

enum class ImGuiTextureOperationType
{
    Create,
    Update,
    Destroy,
};

struct ImGuiTextureRegion
{
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t rowBytes = 0;
    std::vector<std::byte> pixels;
};

struct ImGuiTextureOperation
{
    ImGuiTextureOperationType type = ImGuiTextureOperationType::Create;
    TextureId texture = 0;
    ImTextureFormat format = ImTextureFormat_RGBA32;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<ImGuiTextureRegion> regions;
    // Updates carry a full replacement image as well as dirty-region metadata.
    // The backend versions the texture instead of mutating an image that an
    // older in-flight slot may still sample.
    std::uint32_t fullRowBytes = 0;
    std::vector<std::byte> fullPixels;

    std::size_t PayloadBytes() const noexcept;
};

enum class ImGuiExtractErrorCode
{
    InvalidDrawData,
    InvalidTexture,
    UnsupportedCallback,
};

struct ImGuiExtractError
{
    ImGuiExtractErrorCode code;
    std::string message;
};

class ImGuiRenderPacketExtractor
{
public:
    struct Extraction
    {
        ImGuiRenderPacket packet;
        std::vector<ImGuiTextureOperation> textureOperations;

    private:
        struct Acknowledgement
        {
            ImTextureData* texture = nullptr;
            TextureId logicalId = 0;
            ImGuiTextureOperationType type = ImGuiTextureOperationType::Create;
        };
        std::vector<Acknowledgement> acknowledgements;
        bool committed = false;
        friend class ImGuiRenderPacketExtractor;
    };

    void RegisterCallback(ImDrawCallback callback, RenderCallbackId id);
    void UnregisterCallback(ImDrawCallback callback);
    std::expected<Extraction, ImGuiExtractError> Extract(const ImDrawData& drawData);

    // Call only after ReliableCommands + OptionalDrawPacket was accepted atomically.
    // A rejected submission leaves the ImGui texture state untouched and may retry.
    bool CommitAccepted(Extraction& extraction);
    ImTextureID RegisterDisplaySurface(DisplaySurfaceToken token, std::shared_ptr<const void> lifetimePin);
    bool UnregisterDisplaySurface(ImTextureID textureId) noexcept;

private:
    TextureId AllocateTextureId();
    TextureId ResolveUserTexture(ImTextureID texture);
    TextureId ResolveTextureData(ImTextureData* texture);

    // Keep frontend-created atlas IDs disjoint from backend/user texture IDs.
    TextureId m_NextTexture = TextureId{1} << 63;
    ImTextureID m_NextDisplaySurface = static_cast<ImTextureID>(1) << 62;
    std::unordered_map<ImTextureID, TextureId> m_UserTextures;
    struct DisplaySurfaceRegistration { DisplaySurfaceToken token; std::shared_ptr<const void> pin; };
    std::unordered_map<ImTextureID, DisplaySurfaceRegistration> m_DisplaySurfaces;
    std::unordered_map<ImTextureData*, TextureId> m_TextureData;
    std::unordered_map<ImDrawCallback, RenderCallbackId> m_Callbacks;
};
} // namespace Aether::ImGuiCompat
