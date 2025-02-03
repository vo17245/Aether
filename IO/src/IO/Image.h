#pragma once
#include <string>
#include <expected>
#include <variant>
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
    using ImageData = std::variant<std::monostate, StbImageData>;
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
    };

    static void FreeStbImageData(void* data);

private:
    ImageData m_Data;
};
} // namespace Aether