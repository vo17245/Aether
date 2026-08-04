#pragma once
#include <IO/Image.h>
#include <Render/RHI.h>
#include <Render/Utils.h>
#include <Core/Core.h>

using namespace Aether;
namespace AetherEditor
{

namespace Utils
{
inline std::expected<rhi::Texture2D, std::string> LoadTexture(const std::string& path, PixelFormat format)
{
    auto image = Image::LoadFromFile(path);
    if (!image)
    {
        return std::unexpected<std::string>(std::format("Failed to load image: {}, error: {}", path, image.error()));
    }

    auto texture = rhi::Texture2D::Create({
        .usages = PackFlags(rhi::TextureUsage::TransferDst, rhi::TextureUsage::Sample),
        .pixelFormat = format,
        .width = static_cast<uint32_t>(image->GetWidth()),
        .height = static_cast<uint32_t>(image->GetHeight()),
        .layout = rhi::TextureLayout::Undefined,
    });
    texture.SyncTransitionLayout(rhi::TextureLayout::Undefined, rhi::TextureLayout::TransferDst);

    auto stagingBuffer = rhi::StagingBuffer::Create(image->GetDataSize());
    stagingBuffer.SetData(0, std::span<const uint8_t>(image->GetData(), image->GetDataSize()));
    Render::Utils::SyncUploadTexture2D(stagingBuffer, texture);
    texture.SyncTransitionLayout(rhi::TextureLayout::TransferDst, rhi::TextureLayout::ShaderReadOnly);
    return texture;
}
inline std::expected<rhi::Texture2D, std::string> LoadSrgbTexture(const std::string& path)
{
    return LoadTexture(path, PixelFormat::RGBA8888_SRGB);
}
inline std::expected<rhi::Texture2D, std::string> LoadLinearTexture(const std::string& path)
{
    return LoadTexture(path, PixelFormat::RGBA8888);
}
} // namespace Utils
} // namespace AetherEditor
