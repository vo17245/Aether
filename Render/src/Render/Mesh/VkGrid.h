#pragma once
#include "Core/Base.h"
#include "Grid.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include "Render/VertexBufferLayout.h"
#include "Render/Vulkan/Buffer.h"
#include "vulkan/vulkan_core.h"
#include <unordered_map>
#include <vector>
#include <cassert>
namespace Aether {
inline constexpr VkIndexType GridComponentTypeToVkIndexType(Grid::ComponentType type)
{
    switch (type)
    {
    case Grid::ComponentType::UINT8:
        return VK_INDEX_TYPE_UINT8_EXT;
    case Grid::ComponentType::UINT16:
        return VK_INDEX_TYPE_UINT16;
    case Grid::ComponentType::UINT32:
        return VK_INDEX_TYPE_UINT32;
    default:
        assert(false&&"unknown component type");
        return VK_INDEX_TYPE_MAX_ENUM;
    }
}
class VkGrid
{
public:
    struct PrimitiveVertexResource
    {
        std::vector<uint32_t> buffers;
        std::vector<VertexBufferLayout> layouts;
        // bind vertex buffer时设置的offset
        // 表示buffer偏移bufferOffset字节后可用
        std::vector<uint32_t> bufferOffset;
    };
    struct PrimitiveIndexResource
    {
        uint32_t buffer = 0;
        uint32_t offset = 0;
        uint32_t count = 0;
        Grid::ComponentType type = Grid::ComponentType::NONE;
    };
    struct Primitive
    {
        PrimitiveVertexResource vertexResource;
        std::optional<PrimitiveIndexResource> indexResource;
    };

public:
    static std::optional<VkGrid> Create(vk::GraphicsCommandPool& commandPool, const Grid& Grid)
    {
        VkGrid vkGrid;
        if (!vkGrid.Init(commandPool, Grid))
        {
            return std::nullopt;
        }
        return vkGrid;
    }
    static Ref<VkGrid> CreateRef(vk::GraphicsCommandPool& commandPool, const Grid& Grid)
    {
        Ref<VkGrid> vkGrid = Ref<VkGrid>(new VkGrid());
        if (!vkGrid->Init(commandPool, Grid))
        {
            return nullptr;
        }
        return vkGrid;
    }
    static Scope<VkGrid> CreateScope(vk::GraphicsCommandPool& commandPool, const Grid& Grid)
    {
        Scope<VkGrid> vkGrid = Scope<VkGrid>(new VkGrid());
        if (!vkGrid->Init(commandPool, Grid))
        {
            return nullptr;
        }
        return vkGrid;
    }
    VkGrid(const VkGrid&) = delete;
    VkGrid& operator=(const VkGrid&) = delete;
    VkGrid(VkGrid&&) = default;
    VkGrid& operator=(VkGrid&&) = default;
    size_t GetVertexCount() const
    {
        return m_VertexCount;
    }

private:
    VkGrid() = default;
    bool Init(vk::GraphicsCommandPool& commandPool, const Grid& Grid)
    {
        if (!CreateBuffers(commandPool, Grid))
        {
            return false;
        }
        if (!CreatePrimitive(Grid))
        {
            return false;
        }
        return true;
    }

    bool CreateBuffers(vk::GraphicsCommandPool& commandPool, const Grid& Grid)
    {
        // create staging buffer
        uint32_t maxBufferSize = 0;
        for (const auto& bufferView : Grid.bufferViews)
        {
            maxBufferSize = std::max(maxBufferSize, bufferView.size);
        }
        auto stagingBufferOpt = vk::Buffer::CreateForStaging(maxBufferSize);
        if (!stagingBufferOpt.has_value())
        {
            assert(false&&"failed to create staging buffer");
            return false;
        }
        auto stagingBuffer = std::move(stagingBufferOpt.value());
        // create data for each bufferview and copy data
        for (const auto& bufferView : Grid.bufferViews)
        {
            const auto& buffer = Grid.buffers[bufferView.buffer];
            const size_t size = bufferView.size;
            const size_t offset = bufferView.offset;
            if (bufferView.target == Grid::Target::Index)
            {
                auto vkBufferOpt = vk::Buffer::CreateForIndex(size);
                if (!vkBufferOpt.has_value())
                {
                    assert(false&&"failed to create index buffer");
                    return false;
                }
                auto vkBuffer = std::move(vkBufferOpt.value());
                // copy data
                stagingBuffer.SetData(buffer.data() + offset, size);
                vk::Buffer::SyncCopy(commandPool, stagingBuffer, vkBuffer, size);
                m_Buffers.emplace_back(std::move(vkBuffer));
            }
            else
            {
                auto bufferOpt = vk::Buffer::CreateForVertex(bufferView.size);
                if (!bufferOpt.has_value())
                {
                    assert(false&&"failed to create vertex buffer");
                    return false;
                }
                // copy data
                stagingBuffer.SetData(buffer.data() + offset, size);
                vk::Buffer::SyncCopy(commandPool, stagingBuffer, bufferOpt.value(), size);
                m_Buffers.emplace_back(std::move(bufferOpt.value()));
            }
        }
        return true;
    }

    bool CreatePrimitive(const Grid& Grid)
    {
        std::unordered_map<Grid::Primitive::Attribute, uint32_t> attributeLocation = {
            {Grid::Primitive::Attribute::POSITION, 0},
            {Grid::Primitive::Attribute::NORMAL, 1},
            {Grid::Primitive::Attribute::TEXCOORD, 2},
            {Grid::Primitive::Attribute::TANGENT, 3},
            {Grid::Primitive::Attribute::COLOR, 4},
            {Grid::Primitive::Attribute::BITANGENT, 5},
        };
        auto& primitive = Grid.primitive;
        Primitive vkPrimitive;
        // index
        if (primitive.index.has_value())
        {
            auto accessor = Grid.accessors[primitive.index.value()];
            PrimitiveIndexResource indexResource;

            indexResource.buffer = accessor.bufferView;
            indexResource.offset = accessor.byteOffset;
            indexResource.count = accessor.count;
            indexResource.type = accessor.componentType;
            vkPrimitive.indexResource = std::move(indexResource);
        }
        // vertex
        PrimitiveVertexResource vertexResource;
        // sort attribute
        std::vector<Grid::Primitive::Attribute> attributes;
        for (const auto& [attribute, _] : primitive.attributes)
        {
            attributes.push_back(attribute);
        }
        std::sort(attributes.begin(), attributes.end(), [](Grid::Primitive::Attribute a, Grid::Primitive::Attribute b) {
            return a < b;
        });

        for (auto attribute : attributes)
        {
            auto accessorIndex = primitive.attributes.at(attribute);
            auto& accessor = Grid.accessors[accessorIndex];
            // 这个bufferView是否被使用过了
            {
                auto iter = std::find(vertexResource.buffers.begin(), vertexResource.buffers.end(), accessor.bufferView);
                if (iter == vertexResource.buffers.end())
                {
                    vertexResource.buffers.push_back(accessor.bufferView);
                    VertexBufferLayout layout;
                    layout.SetBinding(vertexResource.buffers.size() - 1);
                    vertexResource.layouts.push_back(std::move(layout));
                }
            }

            // 添加attribute
            auto iter = std::find(vertexResource.buffers.begin(), vertexResource.buffers.end(), accessor.bufferView);
            auto& layout = vertexResource.layouts[iter - vertexResource.buffers.begin()];

            switch (attribute)
            {
            case Grid::Primitive::Attribute::POSITION: {
                BufferElementFormat format = BufferElementFormat::None;
                if (accessor.type == Grid::Type::VEC3 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    format = BufferElementFormat::Vec3f;
                }
                else if (accessor.type == Grid::Type::VEC2 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    format = BufferElementFormat::Vec2f;
                }
                else
                {
                    assert(false&&"unknown format");
                    return false;
                }
                layout.PushAttribute(VertexBufferLayout::Attribute({attributeLocation[attribute], accessor.byteOffset, format}));
            }
            break;
            case Grid::Primitive::Attribute::NORMAL: {
                if (accessor.type == Grid::Type::VEC3 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec3f}));
                }
                else if (accessor.type == Grid::Type::VEC2 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec2f}));
                }
                else
                {
                    assert(false&&"unsupport component type");
                    return false;
                }
            }
            break;
            case Grid::Primitive::Attribute::TEXCOORD: {
                if (accessor.type == Grid::Type::VEC2 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec2f}));
                }
                else
                {
                    assert(false&&"unsupport component type");
                    return false;
                }
            }
            break;
            case Grid::Primitive::Attribute::TANGENT: {
                if (accessor.type == Grid::Type::VEC3 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec3f}));
                }
                else
                {
                    assert(false&&"unsupport component type");
                    return false;
                }
            }
            break;
            case Grid::Primitive::Attribute::COLOR: {
                if (accessor.type == Grid::Type::VEC4 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec4f}));
                }
                else if (accessor.type == Grid::Type::VEC3 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec3f}));
                }
                else
                {
                    assert(false&&"unsupport component type");
                    return false;
                }
            }
            break;
            case Grid::Primitive::Attribute::BITANGENT: {
                if (accessor.type == Grid::Type::VEC3 && accessor.componentType == Grid::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec3f}));
                }
                else
                {
                    assert(false&&"unsupport component type");
                    return false;
                }
            }
            break;
            default:
                assert(false&&"unknown attribute");
                return false;
            }
        }
        vkPrimitive.vertexResource = std::move(vertexResource);
        m_Primitive = std::move(vkPrimitive);
        m_VertexCount = Grid.CalculateVertexCount();
        return true;
    }
    // 把Grid的buffer数据更新到GPU，如果buffer大小不够，会重新创建buffer
    // 只能使用创建时候使用的Grid来更新(或者布局相同的Grid)

public:
    void Update(vk::GraphicsCommandPool& commandPool, const Grid& Grid);
    const std::vector<VertexBufferLayout>& GetVertexBufferLayouts()const
    {
        return m_Primitive.vertexResource.layouts;
    }
    Primitive& GetPrimitive()
    {
        return m_Primitive;
    }
    std::vector<vk::Buffer>& GetBuffers()
    {
        return m_Buffers;
    }

    const Primitive& GetPrimitive() const
    {
        return m_Primitive;
    }
    const std::vector<vk::Buffer>& GetBuffers() const
    {
        return m_Buffers;
    }
    std::vector<VkBuffer> PackPrimitiveVertexBufferHandles()
    {
        std::vector<VkBuffer> buffers;
        for (uint32_t buffer : m_Primitive.vertexResource.buffers)
        {
            buffers.emplace_back(m_Buffers[buffer].GetHandle());
        }
        return buffers;
    }

private:
    std::vector<vk::Buffer> m_Buffers;
    Primitive m_Primitive;
    size_t m_VertexCount = 0;
};
} // namespace Kamui