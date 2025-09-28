#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <future>
#include <limits>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include "VertexBufferLayout.h"
#include "Core/Base.h"
namespace Aether {
/**
 * @note
 *   每个BufferView对应一个VkBuffer
 *   每个primitive确定一组BufferView和布局(意味着一个mesh可能需要多个pipeline)
 */
struct Mesh
{
    enum class Target
    {
        None,
        Vertex,
        Index
    };

    using Buffer = std::vector<uint8_t>;
    struct BufferView
    {
        uint32_t offset = std::numeric_limits<uint32_t>::max();
        uint32_t size = std::numeric_limits<uint32_t>::max();
        uint32_t buffer = std::numeric_limits<uint32_t>::max();
        Target target = Target::None;
    };
    enum class ComponentType : uint32_t
    {
        NONE = 0,
        INT8,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        FLOAT32
    };
    enum class Type : uint32_t
    {
        NONE = 0,
        SCALAR,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4
    };
    static uint32_t ComponentTypeSize(ComponentType type)
    {
        switch (type)
        {
        case ComponentType::INT8:
        case ComponentType::UINT8:
            return 1;
        case ComponentType::INT16:
        case ComponentType::UINT16:
            return 2;
        case ComponentType::INT32:
        case ComponentType::UINT32:
        case ComponentType::FLOAT32:
            return 4;
        default:
            return 0;
        }
    }
    static uint32_t TypeSize(Type type, ComponentType componentType)
    {
        switch (type)
        {
        case Type::SCALAR:
            return ComponentTypeSize(componentType);
        case Type::VEC2:
            return ComponentTypeSize(componentType) * 2;
        case Type::VEC3:
            return ComponentTypeSize(componentType) * 3;
        case Type::VEC4:
            return ComponentTypeSize(componentType) * 4;
        case Type::MAT2:
            return ComponentTypeSize(componentType) * 4;
        case Type::MAT3:
            return ComponentTypeSize(componentType) * 9;
        case Type::MAT4:
            return ComponentTypeSize(componentType) * 16;
        default:
            return 0;
        }
    }
    struct Accessor
    {
        uint32_t bufferView = std::numeric_limits<uint32_t>::max();
        uint32_t byteOffset = std::numeric_limits<uint32_t>::max(); // 在bufferView中的偏移
        ComponentType componentType = ComponentType::NONE;
        uint32_t count = std::numeric_limits<uint32_t>::max();
        Type type = Type::NONE;
    };
    enum class Attribute
        {
            POSITION,
            NORMAL,
            TEXCOORD,
            TANGENT,
            COLOR,
            BITANGENT,

        };
    struct Primitive
    {
        
        std::unordered_map<Attribute, uint32_t> attributes; // attribute -> accessor
        std::optional<uint32_t> index;                      // accessor
        static std::optional<std::string> AttributeToString(Attribute attribute)
        {
            switch (attribute)
            {
            case Attribute::POSITION:
                return "POSITION";
            case Attribute::NORMAL:
                return "NORMAL";
            case Attribute::TEXCOORD:
                return "TEXCOORD";
            case Attribute::TANGENT:
                return "TANGENT";
            case Attribute::COLOR:
                return "COLOR";
            case Attribute::BITANGENT:
                return "BITANGENT";
            default:
                return std::nullopt;
            }
        }
        static std::optional<Attribute> StringToAttribute(const std::string& str)
        {
            if (str == "POSITION")
                return Attribute::POSITION;
            if (str == "NORMAL")
                return Attribute::NORMAL;
            if (str == "TEXCOORD")
                return Attribute::TEXCOORD;
            if (str == "TANGENT")
                return Attribute::TANGENT;
            if (str == "COLOR")
                return Attribute::COLOR;
            if (str == "BITANGENT")
                return Attribute::BITANGENT;
            return std::nullopt;
        }
    };

    std::vector<Buffer> buffers;
    std::vector<BufferView> bufferViews;
    std::vector<Accessor> accessors;
    Primitive primitive;
    size_t CalculateVertexCount() const
    {
        return accessors[primitive.attributes.at(Attribute::POSITION)].count;
    }
    static BufferElementFormat GetBufferElementFormat(Type type,ComponentType componentType)
    {
        switch (type) 
        {
        case Type::SCALAR:
            if(componentType==ComponentType::FLOAT32)
            {
                return BufferElementFormat::Float32;
            }
            else if(componentType==ComponentType::UINT32)
            {
                return BufferElementFormat::UInt32;
            }
            else {
                assert(false&&"not implemented");
                return BufferElementFormat::Float32;
            }
            break;
        case Type::VEC2:
            if(componentType==ComponentType::FLOAT32)
            {
                return BufferElementFormat::Vec2f;
            }
            else {
                assert(false&&"not implemented");
                return BufferElementFormat::Float32;
            }
            break;
        case Type::VEC3:
            if(componentType==ComponentType::FLOAT32)
            {
                return BufferElementFormat::Vec3f;
            }
            else {
                assert(false&&"not implemented");
                return BufferElementFormat::Float32;
            }
            break;
        case Type::VEC4:
            if(componentType==ComponentType::FLOAT32)
            {
                return BufferElementFormat::Vec4f;
            }
            else {
                assert(false&&"not implemented");
                return BufferElementFormat::Float32;
            }
            break;
        default:
            assert(false&&"not implemented");
            return BufferElementFormat::Float32;
        }
    }
    std::vector<VertexBufferLayout> CreateVertexBufferLayouts()const
    {
        // sort attribute by index in enum
        std::vector<Attribute> attributes;
        for(const auto& [attribute,_]:primitive.attributes)
        {
            attributes.push_back(attribute);
        }
        std::sort(attributes.begin(),attributes.end(),[](Attribute a,Attribute b){
            return a<b;
        });
        // create layout for each bufferView used in vertex
        std::vector<VertexBufferLayout> layouts;
        std::vector<int> bufferViewToLayout(bufferViews.size(),-1);
        for(const auto& [index,bufferView]:std::views::enumerate(bufferViews))
        {
            if(bufferView.target==Target::Vertex)
            {
                bufferViewToLayout[index]=layouts.size();
                auto layout=VertexBufferLayout();
                layout.SetBinding(layouts.size());
                layouts.push_back(layout);
            }
        }
        // add attribute to layout
        for(auto&& [index,attribute]:std::views::enumerate(attributes))
        {
            auto& accessorIndex=primitive.attributes.at(attribute);
            auto& accessor=accessors[accessorIndex];
            auto& bufferView=bufferViews[accessor.bufferView];
            auto& layout=layouts[bufferViewToLayout[accessor.bufferView]];
            layout.PushAttribute(VertexBufferLayout::Attribute{
                static_cast<uint32_t>(index),
                accessor.byteOffset,
                GetBufferElementFormat(accessor.type, accessor.componentType)
            });
        }
        return layouts;
    }
};
} // namespace Kamui