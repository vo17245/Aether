#include <Entry/Application.h>
#include <ImGui/Backend/imgui_impl_rendergraph.h>
#include <Window/Layer.h>
#include <SDL3/SDL.h>
#include <array>
#include <cstdio>
#include <stdexcept>

using namespace Aether;
namespace
{
void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

class TestLayer final : public Layer
{
public:
    int frame = 0;
    int callbacks = 0;
    bool sawTextureCreate = false;
    bool sawTextureUpdate = false;
    rhi::Texture2D texture;
    rhi::TextureView view;
    rhi::Sampler sampler;
    ImTextureID id = ImTextureID_Invalid;

    void OnAttach(Window*) override
    {
        rhi::TextureDesc desc{};
        desc.width = desc.height = 2;
        desc.pixelFormat = PixelFormat::RGBA8888;
        desc.usages = PackFlags(rhi::TextureUsage::Sample, rhi::TextureUsage::TransferDst);
        desc.layout = rhi::TextureLayout::Undefined;
        texture = rhi::Texture2D::Create(desc);
        view = texture.CreateImageView({});
        sampler = rhi::Sampler::CreateNearest();
        id = ImGui_ImplRenderGraph_AddTexture(texture, view, sampler);
    }

    void OnImGuiUpdate() override
    {
        ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(290, 110), ImGuiCond_Always);
        ImGui::Begin("RenderGraph test", nullptr, ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted(frame < 3 ? "A" : "Dynamic atlas: abcdefghijklmnopqrstuvwxyz 0123456789");
        ImGui::End();
        auto* draw = ImGui::GetForegroundDrawList();
        draw->AddRectFilled(ImVec2(10, 10), ImVec2(70, 70), IM_COL32(255, 0, 0, 128));
        // Force a new VtxOffset with 16-bit ImDrawIdx, then draw a visible quad.
        for (int i = 0; i < 17000; ++i)
            draw->AddRectFilled(ImVec2(2000, 2000), ImVec2(2001, 2001), IM_COL32_WHITE);
        draw->AddCallback([](const ImDrawList*, const ImDrawCmd* cmd) {
            auto* self = static_cast<TestLayer*>(cmd->UserCallbackData);
            ++self->callbacks;
            Require(ImGui::GetPlatformIO().Renderer_RenderState != nullptr, "Missing renderer callback state");
        }, this);
        draw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        draw->PushClipRect(ImVec2(110, 20), ImVec2(150, 60));
        draw->AddRectFilled(ImVec2(100, 10), ImVec2(160, 70), IM_COL32(0, 255, 0, 255));
        draw->PopClipRect();
        draw->AddImage(ImTextureRef(id), ImVec2(180, 10), ImVec2(240, 70));
    }

    void OnUpload(PendingUploadList& uploads) override
    {
        if (frame == 1)
        {
            const std::array<uint8_t, 16> blue = {0,0,255,255, 0,0,255,255, 0,0,255,255, 0,0,255,255};
            uploads.UploadTexture(blue, &texture, {0, 0, 2, 2}, rhi::TextureLayout::Undefined);
        }
        if (frame == 3)
        {
            const std::array<uint8_t, 4> green = {0,255,0,255};
            uploads.UploadTexture(green, &texture, {1, 1, 1, 1});
        }
        for (auto* atlas : *ImGui::GetDrawData()->Textures)
        {
            sawTextureCreate |= atlas->Status == ImTextureStatus_WantCreate;
            sawTextureUpdate |= atlas->Status == ImTextureStatus_WantUpdates;
        }
    }
};

// Read back the composited image after Entry has waited for all submissions.
void CheckPixels(rhi::Texture2D& texture)
{
    const size_t bytes = static_cast<size_t>(texture.GetWidth()) * texture.GetHeight() * 4;
    auto readback = vk::Buffer::Create(bytes, vk::Buffer::Usage::TransferDst, vk::Buffer::Property::HostVisible);
    auto commands = vk::GraphicsCommandBuffer::Create(vk::GRC::GetGraphicsCommandPool());
    auto fence = vk::Fence::Create();
    Require(readback && commands && fence, "Failed to allocate readback resources");
    commands->BeginSingleTime();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.GetVk().GetHandle();
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCmdPipelineBarrier(commands->GetHandle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageExtent = {texture.GetWidth(), texture.GetHeight(), 1};
    vkCmdCopyImageToBuffer(commands->GetHandle(), barrier.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           readback->GetHandle(), 1, &copy);
    std::swap(barrier.oldLayout, barrier.newLayout);
    std::swap(barrier.srcAccessMask, barrier.dstAccessMask);
    vkCmdPipelineBarrier(commands->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    VkMemoryBarrier hostBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    hostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    hostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(commands->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                         0, 1, &hostBarrier, 0, nullptr, 0, nullptr);
    commands->End();
    commands->BeginSubmit().Fence(*fence).EndSubmit();
    fence->Wait();
    auto* pixels = readback->BeginMap<uint8_t>();
    // Invalidate non-coherent memory before reading the GPU-written pixels.
    readback->Invalidate();
    auto check = [&](int x, int y, std::array<int, 4> expected) {
        const uint8_t* pixel = pixels + (y * texture.GetWidth() + x) * 4;
        for (int c = 0; c < 4; ++c)
            if (std::abs(static_cast<int>(pixel[c]) - expected[c]) > 2)
            {
                std::fprintf(stderr, "Pixel (%d,%d): %u,%u,%u,%u\n", x, y, pixel[0], pixel[1], pixel[2], pixel[3]);
                throw std::runtime_error("Unexpected composited pixel");
            }
    };
    check(30, 30, {192, 89, 127, 255});
    check(120, 30, {0, 255, 0, 255});
    check(105, 15, {128, 179, 255, 255});
    check(190, 20, {0, 0, 255, 255});
    check(230, 60, {0, 255, 0, 255});
    readback->EndMap();
}

class TestApp final : public Application
{
    Window* window = nullptr;
    TestLayer layer;
public:
    void OnInit(Window& value) override
    {
        window = &value;
        window->PushLayer(&layer);
    }
    void OnFrameBegin() override
    {
        ++layer.frame;
        if (layer.frame == 5)
            window->SetSize(400, 300);
        if (layer.frame == 8)
            SDL_MinimizeWindow(window->GetHandle());
        if (layer.frame == 10)
            SDL_RestoreWindow(window->GetHandle());
        if (layer.frame == 18)
            Quit();
    }
    void OnShutdown() override
    {
        Require(layer.callbacks >= 8, "Draw callbacks were not executed");
        Require(layer.sawTextureCreate && layer.sawTextureUpdate, "Dynamic font atlas update not exercised");
        CheckPixels(window->GetFinalTexture(0));
        CheckPixels(window->GetFinalTexture(1));
        ImGui_ImplRenderGraph_RemoveTexture(layer.id);
        std::puts("ImGui RenderGraph GPU tests passed");
    }
    WindowCreateParam MainWindowCreateParam() override
    {
        WindowCreateParam param;
        param.title = "ImGui RenderGraph GPU tests";
        param.width = 320;
        param.height = 240;
        param.imGuiEnableClear = true;
        return param;
    }
};
}
DEFINE_APPLICATION(TestApp);
