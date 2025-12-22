#include "Image.h"
#include "Filesystem/Utils.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <Filesystem/Filesystem.h>
#include <format>
#include <libpng18/png.h>
namespace
{

bool WritePNG_RGBA16(const char* filename, int width, int height,
                     const uint16_t* rgba // RGBA, uint16
)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp)
        return false;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        return false;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return false;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return false;
    }
    // little-endian to big-endian
    png_set_swap(png_ptr);
    png_init_io(png_ptr, fp);

    // -------------------------
    // IHDR
    // -------------------------
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 16,                  // bit depth
                 PNG_COLOR_TYPE_RGBA, // RGBA
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    

    png_write_info(png_ptr, info_ptr);

    // -------------------------
    // Row pointers
    // -------------------------
    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; y++)
    {
        row_pointers[y] = (png_bytep)(rgba + y * width * 4);
    }

    png_write_image(png_ptr, row_pointers.data());
    png_write_end(png_ptr, nullptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return true;
}
} // namespace
namespace Aether
{
std::expected<Image, std::string> Image::LoadFromMemory(const uint8_t* data, size_t size)
{
    int x, y;
    int channels_in_file;
    constexpr int desired_channels = 4;
    uint8_t* res = stbi_load_from_memory(data, size, &x, &y, &channels_in_file, desired_channels);
    if (!res)
    {
        return std::unexpected<std::string>(stbi_failure_reason());
    }
    Image img;
    StbImageData imageData;
    imageData.width = x;
    imageData.height = y;
    imageData.channels = desired_channels;
    imageData.data = res;
    img.m_Data = std::move(imageData);
    return img;
}
void Image::FreeStbImageData(void* data)
{
    stbi_image_free(data);
}
std::expected<Image, std::string> Image::LoadFromFile(const std::string_view path)
{
    auto file = Filesystem::ReadFile(path);
    if (!file)
    {
        return std::unexpected(std::format("Failed to read file: {}", path));
    }
    return LoadFromMemory((uint8_t*)file->data(), file->size());
}
Image Image::CreateRgba8(uint32_t width, uint32_t height)
{
    Image image;
    BasicImageData imageData(width, height, 4, ImageChannelDataType::U8);
    image.m_Data = std::move(imageData);
    return image;
}
bool Image::SaveToPngFile(const char* path)
{
    if (m_Data.index() == 0)
    {
        assert(false && "No image data to save");
        return false; // No data to save
    }
    if (std::holds_alternative<BasicImageData>(m_Data))
    {
        auto& data = std::get<BasicImageData>(m_Data);
        if (data.channelDataType == ImageChannelDataType::U8 && data.channels == 4)
        {
            return stbi_write_png(path, data.width, data.height, data.channels, data.data, data.rowBytes) != 0;
        }
        else if (data.channelDataType == ImageChannelDataType::U16 && data.channels == 4)
        {
            return WritePNG_RGBA16(path, data.width, data.height, reinterpret_cast<const uint16_t*>(data.data));
        }
        else
        {
            assert(false && "unsupport data format for saving png");
            return false;
        }
    }
    else if (std::holds_alternative<StbImageData>(m_Data))
    {
        auto& data = std::get<StbImageData>(m_Data);
        if (data.channelDataType != ImageChannelDataType::U8 || data.channels != 4)
        {
            assert(false && "Only support saving RGBA8 images now");
            return false; // Only support saving RGBA8 images
        }
        return stbi_write_png(path, data.width, data.height, data.channels, data.data, 0) != 0;
    }
    else
    {
        assert(false && "No image data to save");
        return false;
    }
}
Image Image::CreateRgba16(uint32_t width, uint32_t height)
{
    Image image;
    BasicImageData imageData(width, height, 4, ImageChannelDataType::U16);
    image.m_Data = std::move(imageData);
    return image;
}
} // namespace Aether