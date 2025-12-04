#pragma once
#include "Core/Base.h"
#include "Mesh.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include "Render/Mesh/VertexBufferLayout.h"
#include "Render/Vulkan/Buffer.h"
#include "vulkan/vulkan_core.h"
#include <unordered_map>
#include <vector>
#include <cassert>
namespace Aether {
inline constexpr VkIndexType MeshComponentTypeToVkIndexType(Mesh::ComponentType type)
{
    switch (type)
    {
    case Mesh::ComponentType::UINT8:
        return VK_INDEX_TYPE_UINT8_EXT;
    case Mesh::ComponentType::UINT16:
        return VK_INDEX_TYPE_UINT16;
    case Mesh::ComponentType::UINT32:
        return VK_INDEX_TYPE_UINT32;
    default:
        assert(false&&"unknown component type");
        return VK_INDEX_TYPE_MAX_ENUM;
    }
}
class VkMesh
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
        Mesh::ComponentType type = Mesh::ComponentType::NONE;
    };
    struct Primitive
    {
        PrimitiveVertexResource vertexResource;
        std::optional<PrimitiveIndexResource> indexResource;
    };

public:
    static std::optional<VkMesh> Create(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
    {
        VkMesh vkMesh;
        if (!vkMesh.Init(commandPool, Mesh))
        {
            return std::nullopt;
        }
        return vkMesh;
    }
    /**
     * @note caller should delete the pointer
    */
    static VkMesh* CreateRaw(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
    {
        VkMesh* vkMesh = new VkMesh();
        if (!vkMesh->Init(commandPool, Mesh))
        {
            delete vkMesh;
            return nullptr;
        }
        return vkMesh;
    }
    static Ref<VkMesh> CreateRef(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
    {
        Ref<VkMesh> vkMesh = Ref<VkMesh>(new VkMesh());
        if (!vkMesh->Init(commandPool, Mesh))
        {
            return nullptr;
        }
        return vkMesh;
    }
    static Scope<VkMesh> CreateScope(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
    {
        Scope<VkMesh> vkMesh = Scope<VkMesh>(new VkMesh());
        if (!vkMesh->Init(commandPool, Mesh))
        {
            return nullptr;
        }
        return vkMesh;
    }
    VkMesh(const VkMesh&) = delete;
    VkMesh& operator=(const VkMesh&) = delete;
    VkMesh(VkMesh&&) = default;
    VkMesh& operator=(VkMesh&&) = default;
    size_t GetVertexCount() const
    {
        return m_VertexCount;
    }

private:
    VkMesh() = default;
    bool Init(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
    {
        if (!CreateBuffers(commandPool, Mesh))
        {
            return false;
        }
        if (!CreatePrimitive(Mesh))
        {
            return false;
        }
        return true;
    }

    bool CreateBuffers(vk::GraphicsCommandPool& commandPool, const Mesh& Mesh)
    {
        // create staging buffer
        uint32_t maxBufferSize = 0;
        for (const auto& bufferView : Mesh.bufferViews)
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
        for (const auto& bufferView : Mesh.bufferViews)
        {
            const auto& buffer = Mesh.buffers[bufferView.buffer];
            const size_t size = bufferView.size;
            const size_t offset = bufferView.offset;
            if (bufferView.target == Mesh::Target::Index)
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
            m_BufferTargets.push_back(bufferView.target);
        }
        return true;
    }

    bool CreatePrimitive(const Mesh& Mesh)
    {
        std::unordered_map<Mesh::Attribute, uint32_t> attributeLocation = {
            {Mesh::Attribute::POSITION, 0},
            {Mesh::Attribute::NORMAL, 1},
            {Mesh::Attribute::TEXCOORD, 2},
            {Mesh::Attribute::TANGENT, 3},
            {Mesh::Attribute::COLOR, 4},
            {Mesh::Attribute::BITANGENT, 5},
        };
        auto& primitive = Mesh.primitive;
        Primitive vkPrimitive;
        // index
        if (primitive.index.has_value())
        {
            auto accessor = Mesh.accessors[primitive.index.value()];
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
        std::vector<Mesh::Attribute> attributes;
        for (const auto& [attribute, _] : primitive.attributes)
        {
            attributes.push_back(attribute);
        }
        std::sort(attributes.begin(), attributes.end(), [](Mesh::Attribute a, Mesh::Attribute b) {
            return a < b;
        });

        for (auto attribute : attributes)
        {
            auto accessorIndex = primitive.attributes.at(attribute);
            auto& accessor = Mesh.accessors[accessorIndex];
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
            case Mesh::Attribute::POSITION: {
                BufferElementFormat format = BufferElementFormat::None;
                if (accessor.type == Mesh::Type::VEC3 && accessor.componentType == Mesh::ComponentType::FLOAT32)
                {
                    format = BufferElementFormat::Vec3f;
                }
                else if (accessor.type == Mesh::Type::VEC2 && accessor.componentType == Mesh::ComponentType::FLOAT32)
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
            case Mesh::Attribute::NORMAL: {
                if (accessor.type == Mesh::Type::VEC3 && accessor.componentType == Mesh::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec3f}));
                }
                else if (accessor.type == Mesh::Type::VEC2 && accessor.componentType == Mesh::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec2f}));
                }
                else if(accessor.type==Mesh::Type::SCALAR&&accessor.componentType==Mesh::ComponentType::UINT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::UInt32}));
                }
                else
                {
                    assert(false&&"unsupport component type");
                    return false;
                }
            }
            break;
            case Mesh::Attribute::TEXCOORD: {
                if (accessor.type == Mesh::Type::VEC2 && accessor.componentType == Mesh::ComponentType::FLOAT32)
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
            case Mesh::Attribute::TANGENT: {
                if (accessor.type == Mesh::Type::VEC3 && accessor.componentType == Mesh::ComponentType::FLOAT32)
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
            case Mesh::Attribute::COLOR: {
                if (accessor.type == Mesh::Type::VEC4 && accessor.componentType == Mesh::ComponentType::FLOAT32)
                {
                    layout.PushAttribute(VertexBufferLayout::Attribute(
                        {attributeLocation[attribute],
                         accessor.byteOffset,
                         BufferElementFormat::Vec4f}));
                }
                else if (accessor.type == Mesh::Type::VEC3 && accessor.componentType == Mesh::ComponentType::FLOAT32)
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
            case Mesh::Attribute::BITANGENT: {
                if (accessor.type == Mesh::Type::VEC3 && accessor.componentType == Mesh::ComponentType::FLOAT32)
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
        m_VertexCount = Mesh.CalculateVertexCount();
        return true;
    }
    

public:
    // 把Mesh的buffer数据更新到GPU，如果buffer大小不够，会重新创建buffer
    // 只能使用创建时候使用的Mesh来更新(或者布局相同的Mesh)
    void Update(const Mesh& Mesh);
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

        for (size_t i=0;i<m_Buffers.size();i++)
        {
            if(m_BufferTargets[i]!=Mesh::Target::Vertex)
            {
                continue;
            }
            buffers.push_back(m_Buffers[i].GetHandle());
        }
        return buffers;
    }

private:
    std::vector<vk::Buffer> m_Buffers;
    std::vector<Mesh::Target> m_BufferTargets;
    Primitive m_Primitive;
    size_t m_VertexCount = 0;
};
} // namespace Kamui