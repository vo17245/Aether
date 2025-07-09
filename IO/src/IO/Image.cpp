#include "Image.h"
#include "Filesystem/Utils.h"
#include <stb_image.h>
#include <stb_image_write.h>
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
    Image Image::CreateRgba8(uint32_t width,uint32_t height)
    {
        Image image;
        BasicImageData imageData(width,height,4, ImageChannelDataType::U8);
        image.m_Data = std::move(imageData);
        return image;
    }
    bool Image::SaveToPngFile(const char* path)
    {
        if(m_Data.index() == 0)
        {
            return false; // No data to save
        }
        if(std::holds_alternative<BasicImageData>(m_Data))
        {
            auto& data=std::get<BasicImageData>(m_Data);
            if(data.channelDataType != ImageChannelDataType::U8 || data.channels != 4)
            {
                assert(false&&"Only support saving RGBA8 images now");
                return false; // Only support saving RGBA8 images
            }
            return stbi_write_png(path, data.width, data.height, data.channels, data.data, data.rowBytes) != 0;

        }
        else if(std::holds_alternative<StbImageData>(m_Data))
        {
            auto& data = std::get<StbImageData>(m_Data);
            if (data.channelDataType != ImageChannelDataType::U8 || data.channels != 4)
            {
                assert(false && "Only support saving RGBA8 images now");
                return false; // Only support saving RGBA8 images
            }
            return stbi_write_png(path, data.width, data.height, data.channels, data.data, 0) != 0;
        }

    }
}