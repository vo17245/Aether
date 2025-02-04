#pragma once
#include "Render/RenderApi.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Resource/ImageInfo.h"
#include "DynamicStagingBuffer.h"
#include "IO/Image.h"
namespace Aether::UI
{
inline std::expected<DeviceTexture, std::string> LoadTextureToLinear(const std::string_view path,
                                                                     const Resource::ImageInfo& info,
                                                                     DynamicStagingBuffer& stagingBuffer)
{
    // load image
    auto image = Image::LoadFromFile(path);
    if (!image)
    {
        return std::unexpected<std::string>(std::format("Failed to load image: {}, error: {}", path, image.error()));
    }
    PixelFormat imagePixelFormat;
    switch (info.colorSpace)
    {
    case Resource::ImageInfo::ColorSpace::SRGB:
        imagePixelFormat = PixelFormat::RGB888_SRGB;
        break;
    case Resource::ImageInfo::ColorSpace::LINEAR:
        imagePixelFormat = PixelFormat::RGB888;
        break;
    }
    // create texture rgba int8
    auto texture = DeviceTexture::CreateForTexture(image->GetWidth(), image->GetHeight(), imagePixelFormat);
    texture->SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::TransferDst);
    // upload data to staging buffer
    stagingBuffer.SetData(image->GetData(), image->GetDataSize());
    // copy data to texture
    texture->CopyBuffer(stagingBuffer.GetBuffer());
    texture->SyncTransitionLayout(DeviceImageLayout::TransferDst, DeviceImageLayout::Texture);
    return texture;
}
} // namespace Aether::UI