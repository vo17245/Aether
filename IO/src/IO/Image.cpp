#include "Image.h"
#include "Filesystem/Utils.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Filesystem/Filesystem.h>

namespace Aether
{
    std::expected<Image, std::string> Image::LoadFromMemory(const uint8_t* data, size_t size)
    {
        int x,y;
        int channels_in_file;
        constexpr int desired_channels = 4;
        uint8_t* res=stbi_load_from_memory(data, size, &x, &y, &channels_in_file, desired_channels);
        if(!res)
        {
            return std::unexpected<std::string>(stbi_failure_reason());
        }
        Image img;
        StbImageData imageData;
        imageData.width=x;
        imageData.height=y;
        imageData.channels=desired_channels;
        imageData.data=res;
        img.m_Data=std::move(imageData);
        return img;
    }
    void Image::FreeStbImageData(void* data)
    {
        stbi_image_free(data);
    }
    std::expected<Image,std::string> Image::LoadFromFile(const std::string_view path)
    {
        auto file=Filesystem::ReadFile(path);
        if(!file)
        {
            return std::unexpected(std::format("Failed to read file: {}",path));
        }
        return LoadFromMemory((uint8_t*)file->data(), file->size());
    }
}