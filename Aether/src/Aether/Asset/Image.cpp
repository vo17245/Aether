#include "Image.h"
#include "stb/stb_image.h"
#include <assert.h>
AETHER_NAMESPACE_BEGIN



Image::~Image()
{
}

Image::Image(Image&& image)noexcept
{
    m_Data = std::move(image.m_Data);
    m_Height = image.m_Height;
    m_Width = image.m_Width;
    m_Channel = image.m_Channel;
    image.m_Height = 0;
    image.m_Width = 0;
    image.m_Channel = 0;
}

Image::Image(const Image& image)
{
    size_t dataLen = image.m_Channel * image.m_Height * image.m_Width;
 
    m_Data = std::unique_ptr<unsigned char, std::function<void(unsigned char*)>>(new unsigned char[dataLen],ReleaseDataCreatedByCppNewArray);
    memcpy(m_Data.get(), image.m_Data.get(), dataLen);
    m_Height = image.m_Height;
    m_Width = image.m_Width;
    m_Channel = image.m_Channel;
}

std::optional<Image> Image::LoadFromMemDataRGB888(const char* data, size_t width, size_t height)
{
    Image image;
    image.m_Channel = 3;
    image.m_Height = height;
    image.m_Width = width;
    size_t len = width * height * 3;
    image.m_Data = std::unique_ptr<unsigned char, std::function<void(unsigned char*)>>(new unsigned char[len], ReleaseDataCreatedByCppNewArray);
    memcpy(image.m_Data.get(), data, len);
    return image;
}

std::optional<Image> Image::LoadFromMemDataRGBA8888(const char* data, size_t width, size_t height)
{
    Image image;
    image.m_Channel = 4;
    image.m_Height = height;
    image.m_Width = width;
    size_t len = width * height * 4;
    image.m_Data = std::unique_ptr<unsigned char, std::function<void(unsigned char*)>>(new unsigned char[len], ReleaseDataCreatedByCppNewArray);
    memcpy(image.m_Data.get(), data, len);
    return image;
}

std::optional<Image> Image::LoadFromMemDataFormat(unsigned char* data, size_t len, bool flip)
{
    Image image;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char* ptr = stbi_load_from_memory(data,
        static_cast<int>(len), &image.m_Width, &image.m_Height, &image.m_Channel, 0);
    if (ptr == nullptr)
    {
        return std::nullopt;
    }
    image.m_Data = std::unique_ptr<unsigned char, std::function<void(unsigned char*)>>(ptr, ReleaseDataCreatedByStb);
    return image;
}

std::optional<Image> Image::LoadFromFileDataFormat(const std::string& path, bool flip)
{
    Image image;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char* data = stbi_load(path.c_str(), &image.m_Width, &image.m_Height, &image.m_Channel, 0);
    if (data == nullptr)
    {
        return std::nullopt;
    }
    image.m_Data = std::unique_ptr<unsigned char, std::function<void(unsigned char*)>>(data, ReleaseDataCreatedByStb);
    return image;
}

void Image::ReleaseDataCreatedByStb(unsigned char* ptr)
{
    if(ptr!=nullptr)
        STBI_FREE(ptr);
}

void Image::ReleaseDataCreatedByCppNewArray(unsigned char* ptr)
{
    if (ptr != nullptr)
        delete[] ptr;

}




AETHER_NAMESPACE_END


