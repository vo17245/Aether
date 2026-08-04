#include <Render/Mesh/Mesh.h>

namespace Aether
{
static VertexAttributeFormat MeshTypeToVertexAttributeFormat(Mesh::Type type, Mesh::ComponentType componentType)
{
    switch (type)
    {
    case Mesh::Type::SCALAR:
        switch (componentType)
        {
        case Mesh::ComponentType::FLOAT32:
            return VertexAttributeFormat::Float32;
        case Mesh::ComponentType::UINT32:
            return VertexAttributeFormat::UInt32;
        default:
            assert(false && "unsupported component type for SCALAR");
            return VertexAttributeFormat::None;
        }
    case Mesh::Type::VEC2:
        if (componentType == Mesh::ComponentType::FLOAT32)
        {
            return VertexAttributeFormat::Vec2f;
        }
        else
        {
            assert(false && "unsupported component type for VEC2");
            return VertexAttributeFormat::None;
        }
    case Mesh::Type::VEC3:
        if (componentType == Mesh::ComponentType::FLOAT32)
        {
            return VertexAttributeFormat::Vec3f;
        }
        else
        {
            assert(false && "unsupported component type for VEC3");
            return VertexAttributeFormat::None;
        }
    case Mesh::Type::VEC4:
        if (componentType == Mesh::ComponentType::FLOAT32)
        {
            return VertexAttributeFormat::Vec4f;
        }
        else
        {
            assert(false && "unsupported component type for VEC4");
            return VertexAttributeFormat::None;
        }
    default:
        assert(false && "unsupported mesh type");
        return VertexAttributeFormat::None;
    }
}
VertexLayout Mesh::CreateVertexLayout() const
{
    VertexLayout layout;
    std::vector<uint32_t> bufferViewStride(bufferViews.size(), 0);
    for (auto& attribute : primitive.attributes)
    {
        auto accessor = accessors[attribute.second];
        uint32_t elementMaxOffset = accessor.byteOffset + TypeSize(accessor.type, accessor.componentType);
        bufferViewStride[accessor.bufferView] = std::max(bufferViewStride[accessor.bufferView], elementMaxOffset);
        VertexAttribute vertexAttribute{.format =
                                            MeshTypeToVertexAttributeFormat(accessor.type, accessor.componentType),
                                        .offset = accessor.byteOffset,
                                        .bufferViewIndex = accessor.bufferView};
        layout.attributes.push_back(vertexAttribute);
    }
    for (size_t i = 0; i < bufferViews.size(); ++i)
    {
        VertexBufferViewLayout bufferViewLayout{.stride = bufferViewStride[i]};
        layout.bufferViews.push_back(bufferViewLayout);
    }
    return layout;
}

} // namespace Aether
