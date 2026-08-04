#pragma once
#include <Render/RHI.h>
#include <Render/Utils.h>
#include "Resource/ImageInfo.h"
#include "DynamicStagingBuffer.h"
#include "IO/Image.h"

namespace Aether::UI
{
inline std::expected<rhi::Texture2D, std::string> LoadTextureToLinear(const std::string_view path,
                                                                      const Resource::ImageInfo& info,
                                                                      DynamicStagingBuffer& stagingBuffer)
{
    auto image = Image::LoadFromFile(path);
    if (!image)
    {
        return std::unexpected<std::string>(std::format("Failed to load image: {}, error: {}", path, image.error()));
    }

    PixelFormat imagePixelFormat = PixelFormat::RGB888;
    switch (info.colorSpace)
    {
    case Resource::ColorSpace::SRGB:
        imagePixelFormat = PixelFormat::RGB888_SRGB;
        break;
    case Resource::ColorSpace::LINEAR:
        imagePixelFormat = PixelFormat::RGB888;
        break;
    }

    auto texture = rhi::Texture2D::Create({.usages = PackFlags(rhi::TextureUsage::TransferDst, rhi::TextureUsage::Sample),
                                          .pixelFormat = imagePixelFormat,
                                          .width = static_cast<uint32_t>(image->GetWidth()),
                                          .height = static_cast<uint32_t>(image->GetHeight()),
                                          .layout = rhi::TextureLayout::Undefined});
    texture.SyncTransitionLayout(rhi::TextureLayout::Undefined, rhi::TextureLayout::TransferDst);
    stagingBuffer.SetData(image->GetData(), image->GetDataSize());
    Render::Utils::SyncUploadTexture2D(stagingBuffer.GetBuffer(), texture);
    texture.SyncTransitionLayout(rhi::TextureLayout::TransferDst, rhi::TextureLayout::ShaderReadOnly);
    return texture;
}
} // namespace Aether::UI
