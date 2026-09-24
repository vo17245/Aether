#include <IO/Image.h>
#include <exr.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
std::vector<uint8_t> MakeExr(const float* pixels, int width, int channels, bool layered = false)
{
    exr_part part{};
    if (exr_rgba_float_to_part(nullptr, pixels, width, 1, channels, EXR_PIXEL_HALF, &part) != EXR_SUCCESS)
        return {};
    if (layered)
    {
        for (int i = 0; i < channels; ++i)
        {
            const std::string name = std::string("beauty.") + part.header.channels[i].name;
            std::memcpy(part.header.channels[i].name, name.c_str(), name.size() + 1);
        }
    }

    exr_image source{};
    source.num_parts = 1;
    source.parts = &part;
    void* bytes = nullptr;
    size_t size = 0;
    const exr_result result = exr_save_to_memory(&bytes, &size, nullptr, &source, EXR_COMPRESSION_NONE);
    exr_part_free(nullptr, &part);
    if (result != EXR_SUCCESS)
        return {};

    const auto* first = static_cast<const uint8_t*>(bytes);
    std::vector<uint8_t> file(first, first + size);
    std::free(bytes);
    return file;
}

bool CheckImage(const Aether::Image& image, const std::vector<float>& expected, int width)
{
    if (image.GetWidth() != width || image.GetHeight() != 1 || image.GetChannels() != 4 ||
        image.GetChannelDataType() != Aether::ImageChannelDataType::F32 ||
        image.GetRowBytes() != expected.size() * sizeof(float) ||
        image.GetDataSize() != expected.size() * sizeof(float))
        return false;

    const float* pixels = image.GetDataAs<float>();
    for (size_t i = 0; i < expected.size(); ++i)
    {
        if (pixels[i] != expected[i])
            return false;
    }
    return true;
}
}

int main()
{
    const std::array<float, 6> rgb = {0.5f, 1.0f, 2.0f, 4.0f, 0.25f, 0.125f};
    const auto rgbExr = MakeExr(rgb.data(), 2, 3);
    const auto rgbImage = Aether::Image::LoadFromMemory(rgbExr.data(), rgbExr.size());
    if (!rgbImage || !CheckImage(*rgbImage, {0.5f, 1.0f, 2.0f, 1.0f, 4.0f, 0.25f, 0.125f, 1.0f}, 2))
    {
        std::cerr << "RGB EXR memory load failed\n";
        return 1;
    }

    const std::filesystem::path path = std::filesystem::current_path() / "IO.Tests.exr";
    {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(rgbExr.data()), static_cast<std::streamsize>(rgbExr.size()));
    }
    const auto fileImage = Aether::Image::LoadFromFile(path.string());
    std::filesystem::remove(path);
    if (!fileImage || !CheckImage(*fileImage, {0.5f, 1.0f, 2.0f, 1.0f, 4.0f, 0.25f, 0.125f, 1.0f}, 2))
    {
        std::cerr << "RGB EXR file load failed\n";
        return 1;
    }

    const std::array<float, 4> rgba = {2.0f, 1.0f, 0.5f, 0.25f};
    const auto rgbaExr = MakeExr(rgba.data(), 1, 4);
    const auto rgbaImage = Aether::Image::LoadFromMemory(rgbaExr.data(), rgbaExr.size());
    if (!rgbaImage || !CheckImage(*rgbaImage, {2.0f, 1.0f, 0.5f, 0.25f}, 1))
    {
        std::cerr << "RGBA EXR memory load failed\n";
        return 1;
    }

    const auto layeredExr = MakeExr(rgb.data(), 2, 3, true);
    const auto layeredImage = Aether::Image::LoadFromMemory(layeredExr.data(), layeredExr.size());
    if (!layeredImage || !CheckImage(*layeredImage, {0.5f, 1.0f, 2.0f, 1.0f, 4.0f, 0.25f, 0.125f, 1.0f}, 2))
    {
        std::cerr << "layered RGB EXR memory load failed\n";
        return 1;
    }

    const float gray = 0.5f;
    const auto grayExr = MakeExr(&gray, 1, 1);
    const auto grayImage = Aether::Image::LoadFromMemory(grayExr.data(), grayExr.size());
    if (!grayImage || !CheckImage(*grayImage, {0.5f, 0.5f, 0.5f, 1.0f}, 1))
    {
        std::cerr << "grayscale EXR memory load failed\n";
        return 1;
    }

    const std::array<uint8_t, 8> corruptExr = {0x76, 0x2f, 0x31, 0x01, 0, 0, 0, 0};
    if (Aether::Image::LoadFromMemory(corruptExr.data(), corruptExr.size()))
    {
        std::cerr << "corrupt EXR was accepted\n";
        return 1;
    }
    return 0;
}
