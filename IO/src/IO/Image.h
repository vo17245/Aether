#pragma once
#include <string>
#include <expected>
#include <variant>
#include <cstdint>
#include <cassert>
namespace Aether
{

enum class ImageChannelDataType
{
    U8,
    U16,
    F32
};
class Image
{
public:
    static std::expected<Image, std::string> LoadFromMemory(const uint8_t* data, size_t size);
    static std::expected<Image,std::string> LoadFromFile(const std::string_view path);
    static Image CreateRgba8(uint32_t width,uint32_t height);
    bool SaveToPngFile(const char* path);
    bool SaveToPngFile(const std::string& path)
    {
        return SaveToPngFile(path.c_str());
    }
    bool Empty() const
    {
        return m_Data.index() == 0;
    }
    int GetWidth() const
    {
        return std::visit(GetWidthImpl{}, m_Data);
    }
    int GetHeight() const
    {
        return std::visit(GetHeightImpl{}, m_Data);
    }
    int GetChannels() const
    {
        return std::visit(GetChannelsImpl{}, m_Data);
    }
    uint8_t* GetData() const
    {
        return std::visit(GetDataImpl{}, m_Data);
    
    
    }
    /**
     * @brief means stride in bytes / bytes in one row 
    */
    size_t GetRowBytes() const
    {
        return GetWidth() * GetChannels();
    }
    size_t GetDataSize()const
    {
        return GetRowBytes() * GetHeight();
    }


private:
    struct StbImageData
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        ImageChannelDataType channelDataType = ImageChannelDataType::U8;
        uint8_t* data = nullptr;
        void Destroy()
        {
            if (data)
                FreeStbImageData(data);
            data = nullptr;
        }
        StbImageData()=default;
        StbImageData(const StbImageData& other) = delete;
        StbImageData& operator=(const StbImageData& other) = delete;
        StbImageData(StbImageData&& other) noexcept
        {
            width = other.width;
            height = other.height;
            channels = other.channels;
            channelDataType = other.channelDataType;
            data = other.data;
            other.data = nullptr;
        }
        StbImageData& operator=(StbImageData&& other) noexcept
        {
            if (this != &other)
            {
                Destroy();
                width = other.width;
                height = other.height;
                channels = other.channels;
                channelDataType = other.channelDataType;
                data = other.data;
                other.data = nullptr;
            }
            return *this;
        }
        ~StbImageData()
        {
            if (data)
                FreeStbImageData(data);
        }
    };
    struct BasicImageData
    {
        BasicImageData()=default;
        BasicImageData(const BasicImageData&)=delete;
        BasicImageData& operator=(const BasicImageData&)=delete;
        BasicImageData(BasicImageData&& other) noexcept
        {
            width = other.width;
            height = other.height;
            channels = other.channels;
            channelDataType = other.channelDataType;
            rowBytes = other.rowBytes;
            data = other.data;
            other.data = nullptr;
        }
        BasicImageData& operator=(BasicImageData&& other) noexcept
        {
            if (this != &other)
            {
                width = other.width;
                height = other.height;
                channels = other.channels;
                channelDataType = other.channelDataType;
                rowBytes = other.rowBytes;
                delete[] data; // Free existing data
                data = other.data;
                other.data = nullptr;
            }
            return *this;
        }
        ImageChannelDataType channelDataType = ImageChannelDataType::U8;
        uint32_t width;
        uint32_t height;
        uint32_t channels = 4; // Default to RGBA
        uint32_t rowBytes = 0; // Bytes in one row
        uint8_t* data=nullptr;
        BasicImageData(uint32_t _width,uint32_t _height,uint32_t _channels,ImageChannelDataType _channelDataType)
        :width(_width),height(_height),channels(_channels),channelDataType(_channelDataType)
        {
            uint32_t pixelSize;
            switch (channelDataType)
            {
                case ImageChannelDataType::U8:
                    pixelSize = _channels;
                    break;
                case ImageChannelDataType::U16:
                    pixelSize = _channels * 2; // 2 bytes per channel
                    break;
                case ImageChannelDataType::F32:
                    pixelSize = _channels * 4; // 4 bytes per channel
                    break;
                default:
                assert(false&&"Unsupported channel data type");
            }
            data=new uint8_t[width * height * 4]; // Assuming 4 channels (RGBA
            rowBytes = width * pixelSize; // Bytes in one row
        }
        ~BasicImageData()
        {
            if (data)
            {
                delete[] data;
            }
        }
    };
    using ImageData = std::variant<std::monostate, StbImageData,BasicImageData>;
    struct GetWidthImpl
    {
        int operator()(const std::monostate&) const
        {
            return 0;
        }
        int operator()(const StbImageData& data) const
        {
            return data.width;
        }
        int operator()(const BasicImageData& data)const 
        {
            return data.width;
        }
    };
    struct GetHeightImpl
    {
        int operator()(const std::monostate&) const
        {
            return 0;
        }
        int operator()(const StbImageData& data) const
        {
            return data.height;
        }
        int operator()(const BasicImageData& data) const
        {
            return data.height;
        }
    };
    struct GetChannelsImpl
    {
        int operator()(const std::monostate&) const
        {
            return 0;
        }
        int operator()(const StbImageData& data) const
        {
            return data.channels;
        }
        int operator()(const BasicImageData& data) const
        {
            return data.channels;
        }
    };
    struct GetDataImpl
    {
        uint8_t* operator()(const std::monostate&) const
        {
            return nullptr;
        }
        uint8_t* operator()(const StbImageData& data) const
        {
            return data.data;
        }
        uint8_t* operator()(const BasicImageData& data) const
        {
            return data.data;
        }
    };
    struct GetStrideImpl
    {
        size_t operator()(const std::monostate&) const
        {
            return 0;
        }
        size_t operator()(const StbImageData& data) const
        {
            uint32_t pixelSize;
            switch (data.channelDataType)
            {
                case ImageChannelDataType::U8:
                    pixelSize = data.channels;
                    break;
                case ImageChannelDataType::U16:
                    pixelSize = data.channels * 2; // 2 bytes per channel
                    break;
                case ImageChannelDataType::F32:
                    pixelSize = data.channels * 4; // 4 bytes per channel
                    break;
                default:
                    assert(false && "Unsupported channel data type");
            }
            return data.width * pixelSize;
        }
        size_t operator()(const BasicImageData& data) const
        {
            return data.rowBytes;
        }
    };
    

    static void FreeStbImageData(void* data);

private:
    ImageData m_Data;
};
} // namespace Aether