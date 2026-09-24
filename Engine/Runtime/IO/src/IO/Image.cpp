#include "Image.h"
#include "Filesystem/Utils.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <exr.h>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
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
    if (!data || size == 0)
        return std::unexpected("Empty image data");

    if (exr_is_exr_memory(data, size))
    {
        exr_image decoded{};
        const exr_result loadResult = exr_load_from_memory(data, size, nullptr, &decoded);
        std::unique_ptr<exr_image, decltype(&exr_image_free)> decodedGuard(&decoded, &exr_image_free);
        if (loadResult != EXR_SUCCESS)
            return std::unexpected(std::format("Failed to decode EXR: {}", exr_result_string(loadResult)));

        const exr_part* selected = nullptr;
        int red = -1, green = -1, blue = -1, alpha = -1, gray = -1;
        bool luminanceChroma = false;
        for (int partIndex = 0; partIndex < decoded.num_parts; ++partIndex)
        {
            const exr_part& part = decoded.parts[partIndex];
            if (part.is_deep || !part.images)
                continue;

            if (exr_part_is_luminance_chroma(&part))
            {
                selected = &part;
                luminanceChroma = true;
                break;
            }

            const auto findChannel = [&part](std::string_view name) {
                for (int i = 0; i < part.header.num_channels; ++i)
                {
                    if (name == part.header.channels[i].name)
                        return i;
                }
                return -1;
            };

            red = findChannel("R");
            green = findChannel("G");
            blue = findChannel("B");
            alpha = findChannel("A");
            if (red < 0 || green < 0 || blue < 0)
            {
                for (int i = 0; i < part.header.num_channels; ++i)
                {
                    std::string_view name = part.header.channels[i].name;
                    if (!name.ends_with(".R"))
                        continue;
                    const std::string_view layer = name.substr(0, name.size() - 1);
                    red = i;
                    green = findChannel(std::string(layer) + "G");
                    blue = findChannel(std::string(layer) + "B");
                    alpha = findChannel(std::string(layer) + "A");
                    if (green >= 0 && blue >= 0)
                        break;
                }
            }
            if (red >= 0 && green >= 0 && blue >= 0)
            {
                selected = &part;
                break;
            }

            gray = findChannel("Y");
            if (gray < 0 && part.header.num_channels == 1)
                gray = 0;
            if (gray >= 0)
            {
                selected = &part;
                break;
            }
        }
        if (!selected)
            return std::unexpected("EXR has no supported RGB or grayscale image part");

        float* convertedData = nullptr;
        int width = 0, height = 0, channels = 0;
        exr_result convertResult;
        if (luminanceChroma)
        {
            convertResult = exr_part_yc_to_rgba_float(nullptr, selected, &convertedData, &width, &height);
            channels = 4;
        }
        else
        {
            convertResult = exr_part_to_rgba_float(nullptr, selected, &convertedData, &width, &height, &channels);
        }
        std::unique_ptr<float, decltype(&std::free)> converted(convertedData, &std::free);
        if (convertResult != EXR_SUCCESS)
            return std::unexpected(std::format("Failed to convert EXR pixels: {}", exr_result_string(convertResult)));
        if (width <= 0 || height <= 0 ||
            static_cast<size_t>(width) > std::numeric_limits<size_t>::max() / (4 * sizeof(float)) / static_cast<size_t>(height))
            return std::unexpected("EXR dimensions are too large");

        Image image;
        BasicImageData imageData(static_cast<uint32_t>(width), static_cast<uint32_t>(height), 4, ImageChannelDataType::F32);
        float* output = reinterpret_cast<float*>(imageData.data);
        const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
        for (size_t i = 0; i < pixelCount; ++i)
        {
            const float* pixel = converted.get() + i * static_cast<size_t>(channels);
            float* rgba = output + i * 4;
            if (luminanceChroma)
            {
                std::memcpy(rgba, pixel, 4 * sizeof(float));
            }
            else if (gray >= 0)
            {
                rgba[0] = rgba[1] = rgba[2] = pixel[gray];
                rgba[3] = alpha >= 0 ? pixel[alpha] : 1.0f;
            }
            else
            {
                rgba[0] = pixel[red];
                rgba[1] = pixel[green];
                rgba[2] = pixel[blue];
                rgba[3] = alpha >= 0 ? pixel[alpha] : 1.0f;
            }
        }
        image.m_Data = std::move(imageData);
        return image;
    }

    if (size > static_cast<size_t>(std::numeric_limits<int>::max()))
        return std::unexpected("Image data is too large for stb_image");
    int x, y;
    int channels_in_file;
    constexpr int desired_channels = 4;
    uint8_t* res = stbi_load_from_memory(data, static_cast<int>(size), &x, &y, &channels_in_file, desired_channels);
    if (!res)
        return std::unexpected<std::string>(stbi_failure_reason());
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