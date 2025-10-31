#pragma once
#include <UI/Render/LoadTextureToLinear.h>
using namespace Aether;
namespace AetherEditor
{

namespace Utils
{
    inline std::expected<DeviceTexture,std::string> LoadSrgbTexture(const std::string& path)
    {
        // load image
        auto image = Image::LoadFromFile(path);
        if (!image)
        {
            return std::unexpected<std::string>(std::format("Failed to load image: {}, error: {}", path, image.error()));
        }
        PixelFormat imagePixelFormat;
        imagePixelFormat = PixelFormat::RGBA8888_SRGB;
    
        // create texture rgba int8
        auto texture = DeviceTexture::CreateForTexture(image->GetWidth(), image->GetHeight(), imagePixelFormat);
        texture->SyncTransitionLayout(DeviceImageLayout::Undefined, DeviceImageLayout::TransferDst);
        // upload data to staging buffer
        auto stagingBuffer=DeviceBuffer::CreateForStaging(image->GetDataSize());
        stagingBuffer.SetData(image->GetData(), image->GetDataSize());
        // copy data to texture
        texture->CopyBuffer(stagingBuffer);
        texture->SyncTransitionLayout(DeviceImageLayout::TransferDst, DeviceImageLayout::Texture);
        return texture;
    }
}
}