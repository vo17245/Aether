#include <ImGui/Compat/ImGuiRenderPacket.h>
#include <Core/DisplaySurfaceToken.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace Aether::ImGuiCompat;

namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void Callback(const ImDrawList*, const ImDrawCmd*) {}

ImDrawData MakeData(ImDrawList& list, ImVector<ImTextureData*>& textures)
{
    ImDrawData data;
    data.Valid = true;
    data.DisplayPos = {3.0f, 4.0f};
    data.DisplaySize = {100.0f, 50.0f};
    data.FramebufferScale = {2.0f, 2.0f};
    data.CmdLists.push_back(&list);
    data.CmdListsCount = 1;
    data.TotalVtxCount = list.VtxBuffer.Size;
    data.TotalIdxCount = list.IdxBuffer.Size;
    data.Textures = &textures;
    return data;
}

void TestOwnedPacketAndTextureCommit()
{
    ImTextureData texture;
    texture.Create(ImTextureFormat_RGBA32, 2, 2);
    auto* pixels = static_cast<unsigned char*>(texture.GetPixels());
    for (int index = 0; index < texture.GetSizeInBytes(); ++index) pixels[index] = static_cast<unsigned char>(index);
    texture.SetStatus(ImTextureStatus_WantCreate);

    ImDrawList list(nullptr);
    list.VtxBuffer.push_back({{1.0f, 2.0f}, {0.25f, 0.5f}, 0xff102030u});
    list.IdxBuffer.push_back(0);
    ImDrawCmd draw;
    draw.ClipRect = {1, 2, 9, 10};
    draw.TexRef = texture.GetTexRef();
    draw.ElemCount = 1;
    list.CmdBuffer.push_back(draw);
    ImVector<ImTextureData*> textures;
    textures.push_back(&texture);
    ImDrawData data = MakeData(list, textures);

    ImGuiRenderPacketExtractor extractor;
    auto extracted = extractor.Extract(data);
    Check(extracted.has_value(), "valid draw data was rejected");
    auto& extraction = *extracted;
    Check(texture.Status == ImTextureStatus_WantCreate && texture.GetTexID() == ImTextureID_Invalid,
          "extract acknowledged texture before queue acceptance");
    Check(extraction.packet.vertices.size() == 1 && extraction.packet.indices.size() == 1 &&
              extraction.packet.commands.size() == 1,
          "draw packet did not copy buffers");
    Check(extraction.textureOperations.size() == 1 &&
              extraction.textureOperations[0].type == ImGuiTextureOperationType::Create &&
              extraction.textureOperations[0].regions[0].pixels.size() == 16,
          "create operation did not own the full texture");
    const auto logicalId = extraction.packet.commands[0].texture;
    Check(logicalId != 0 && logicalId == extraction.textureOperations[0].texture,
          "draw and texture operation use different logical IDs");

    std::memset(texture.Pixels, 0xff, texture.GetSizeInBytes());
    list.VtxBuffer.clear();
    list.IdxBuffer.clear();
    list.CmdBuffer.clear();
    Check(extraction.packet.vertices[0].position[0] == 1.0f &&
              std::to_integer<unsigned char>(extraction.textureOperations[0].regions[0].pixels[1]) == 1,
          "packet borrowed original ImGui memory");

    Check(extractor.CommitAccepted(extraction), "accepted extraction did not commit");
    Check(!extractor.CommitAccepted(extraction), "texture acknowledgement committed twice");
    Check(texture.Status == ImTextureStatus_OK && texture.GetTexID() == logicalId,
          "accepted texture state was not acknowledged");

    texture.SetStatus(ImTextureStatus_WantDestroy);
    ImDrawList empty(nullptr);
    ImDrawData destroyData = MakeData(empty, textures);
    auto destroy = extractor.Extract(destroyData);
    Check(destroy && destroy->textureOperations.size() == 1 &&
              destroy->textureOperations[0].type == ImGuiTextureOperationType::Destroy,
          "texture destroy was not extracted reliably");
    Check(texture.Status == ImTextureStatus_WantDestroy, "destroy acknowledged before acceptance");
    Check(extractor.CommitAccepted(*destroy), "destroy acknowledgement failed");
    Check(texture.Status == ImTextureStatus_Destroyed && texture.GetTexID() == ImTextureID_Invalid,
          "destroy acknowledgement did not clear frontend state");
}

void TestCallbackProtocol()
{
    ImDrawList list(nullptr);
    ImDrawCmd command;
    int payload = 73;
    command.UserCallback = Callback;
    command.UserCallbackData = &payload;
    command.UserCallbackDataSize = sizeof(payload);
    list.CmdBuffer.push_back(command);
    ImVector<ImTextureData*> textures;
    ImDrawData data = MakeData(list, textures);
    ImGuiRenderPacketExtractor extractor;

    auto unsupported = extractor.Extract(data);
    Check(!unsupported && unsupported.error().code == ImGuiExtractErrorCode::UnsupportedCallback,
          "unregistered raw callback was not rejected");
    extractor.RegisterCallback(Callback, 9);
    auto extracted = extractor.Extract(data);
    Check(extracted && extracted->packet.commands.size() == 1, "registered callback extraction failed");
    const auto& copied = extracted->packet.commands[0];
    Check(copied.type == ImGuiPacketCommandType::Callback && copied.callback == 9 &&
              copied.callbackPayload.size() == sizeof(payload),
          "registered callback metadata was not copied");
    payload = 0;
    int copiedPayload = 0;
    std::memcpy(&copiedPayload, copied.callbackPayload.data(), sizeof(copiedPayload));
    Check(copiedPayload == 73, "callback payload borrowed caller memory");

    list.CmdBuffer[0] = ImDrawCmd();
    list.CmdBuffer[0].UserCallback = ImDrawCallback_ResetRenderState;
    auto reset = extractor.Extract(data);
    Check(reset && reset->packet.commands[0].type == ImGuiPacketCommandType::ResetRenderState,
          "ResetRenderState was not preserved");
}

void TestDisplaySurfaceToken()
{
    ImDrawList list(nullptr);
    list.VtxBuffer.push_back({{1.0f, 2.0f}, {0.0f, 0.0f}, 0xffffffffu});
    list.IdxBuffer.push_back(0);

    ImGuiRenderPacketExtractor extractor;
    const Aether::DisplaySurfaceToken expected{.id = 41, .generation = 7};
    const ImTextureID texture = extractor.RegisterDisplaySurface(expected);
    Check(texture != ImTextureID_Invalid, "display surface handle allocation failed");
    ImDrawCmd draw;
    draw.ClipRect = {0, 0, 20, 20};
    draw.TexRef = ImTextureRef(texture);
    draw.ElemCount = 1;
    list.CmdBuffer.push_back(draw);
    ImVector<ImTextureData*> textures;
    ImDrawData data = MakeData(list, textures);

    auto extracted = extractor.Extract(data);
    Check(extracted && extracted->packet.commands.size() == 1,
          "display surface draw command extraction failed");
    const auto& command = extracted->packet.commands.front();
    Check(command.displaySurface && *command.displaySurface == expected && command.texture == 0,
          "display surface token was mixed with a numeric texture ID");
    Check(extractor.UnregisterDisplaySurface(texture), "registered display surface was not removed");
}
} // namespace

int main()
{
    try
    {
        TestOwnedPacketAndTextureCommit();
        TestCallbackProtocol();
        TestDisplaySurfaceToken();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
    return 0;
}
