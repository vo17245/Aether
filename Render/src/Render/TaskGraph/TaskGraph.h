#pragma once
#include <vector>
#include <Render/RenderApi.h>
namespace Aether::TaskGraph
{
    template<typename T>
    struct Hash 
    {
        size_t operator()(const T& value) const 
        {
            return std::hash<T>()(value);
        }
    };
    struct IResource 
    {
        virtual ~IResource()=default;
        virtual bool Realize()=0;
        virtual bool IsRealized() const=0;
    };
    enum class BufferType:uint32_t 
    {
        SSBO,
        UBO,
        VBO,
        Staging
    };  
    struct BufferDesc 
    {   
        BufferType type;
        size_t size; // in bytes
        bool operator==(const BufferDesc& other) const 
        {
            return type == other.type && size == other.size;
        }

    };
    template<>
    struct Hash<BufferDesc>
    {
        size_t operator()(const BufferDesc& desc)
        {
            return std::hash<uint32_t>()(static_cast<uint32_t>(desc.type)) ^ std::hash<size_t>()(desc.size);
        }
    };
    class Buffer:IResource 
    {
        BufferDesc desc;
        DeviceBuffer deviceBuffer;
        bool Realize()override 
        {
            switch (desc.type) 
            {
            case BufferType::Staging:
                deviceBuffer = DeviceBuffer::CreateForStaging(desc.size);
                break;
            case BufferType::SSBO:
                deviceBuffer = DeviceBuffer::CreateForSSBO(desc.size);
                break;
            case BufferType::UBO:
                deviceBuffer = DeviceBuffer::CreateForUniform(desc.size);
                break;
            case BufferType::VBO:
                deviceBuffer = DeviceBuffer::CreateForVBO(desc.size);
                break;
            default:
                assert(false && "Unknown buffer type");
                return false;
            }
            return !deviceBuffer.Empty();
        }
        bool IsRealized() const override 
        {
            return !deviceBuffer.Empty();
        }
        
    };
    enum class TextureUsage:uint32_t
    {
        Download=Bit(0),
        Upload=Bit(1),
        Sample=Bit(2),
        ColorAttachment=Bit(3),
        DepthAttachment=Bit(4),
    };
    
    
    struct TextureDesc
    {
        TextureUsage usage;
        PixelFormat pixelFormat;
        uint32_t width;
        uint32_t height;
        bool operator==(const TextureDesc& other) const
        {
            return usage == other.usage && pixelFormat == other.pixelFormat && width == other.width && height == other.height;
        }
    };
    template<>
    struct Hash<TextureDesc>
    {
        size_t operator()(const TextureDesc& desc) const
        {
            return std::hash<uint32_t>()(static_cast<uint32_t>(desc.usage)) ^
                   std::hash<uint32_t>()(static_cast<uint32_t>(desc.pixelFormat)) ^
                   std::hash<uint32_t>()(desc.width) ^
                   std::hash<uint32_t>()(desc.height);
        }
    };
    struct Texture:IResource
    {
        TextureDesc desc;
        DeviceTexture deviceTexture;
        bool Realize() override
        {
            switch (desc.usage)
            {
            }
            return !deviceTexture.Empty();
        }
        bool IsRealized() const override
        {
            return !deviceTexture.Empty();
        }
    };



    class TaskGraph
    {
    public:
        template<typename T>
        T* AddDeviceTask()
        {

        }
        
    };
}