#include "Geometry.h"
#include <cstdint>
namespace Aether
{
Mesh Geometry::CreateSphere(float radius, uint32_t segments)
{
    std::vector<Vec3> positions;
    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i <= segments; i++)
    {
        float theta = i * Math::PI / segments;
        for (uint32_t j = 0; j <= segments; j++)
        {
            float phi = j * 2 * Math::PI / segments;
            Vec3 position = Vec3(
                radius * sin(theta) * cos(phi),
                radius * cos(theta),
                radius * sin(theta) * sin(phi));
            positions.push_back(position);
        }
    }
    for (uint32_t i = 0; i < segments; i++)
    {
        for (uint32_t j = 0; j < segments; j++)
        {
            uint32_t i0 = i * (segments + 1) + j;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = i0 + segments + 1;
            uint32_t i3 = i2 + 1;
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i3);
            indices.push_back(i2);
        }
    }
    std::vector<Vec3> normals(positions.size());
    for (size_t i = 0; i < normals.size(); i++)
    {
        normals[i] = positions[i].normalized();
    }

    Mesh Mesh;
    uint32_t positionBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back(
        Mesh::Buffer(
            (uint8_t*)positions.data(),
            (uint8_t*)(positions.data()) + positions.size() * sizeof(Vec3)));
    uint32_t normalBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back(
        Mesh::Buffer(
            (uint8_t*)normals.data(),
            (uint8_t*)(normals.data()) + normals.size() * sizeof(Vec3)));
    uint32_t indexBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back(Mesh::Buffer(
        (uint8_t*)indices.data(),
        (uint8_t*)(indices.data()) + indices.size() * sizeof(uint32_t)));
    uint32_t psitionBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back(Mesh::BufferView{
        0,
        static_cast<uint32_t>(positions.size() * sizeof(Vec3)),
        positionBufferIndex,
        Mesh::Target::Vertex});
    uint32_t normalBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back(Mesh::BufferView{
        0,
        static_cast<uint32_t>(normals.size() * sizeof(Vec3)),
        normalBufferIndex,
        Mesh::Target::Vertex});
    uint32_t indexBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back(Mesh::BufferView{
        0,
        static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
        indexBufferIndex,
        Mesh::Target::Index});
    uint32_t positionAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back(Mesh::Accessor{
        psitionBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        8,
        Mesh::Type::VEC3});
    uint32_t normalAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back(Mesh::Accessor{
        normalBufferViewIndex,
        0,
        Mesh::ComponentType::FLOAT32,
        8,
        Mesh::Type::VEC3});
    uint32_t indexAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back(Mesh::Accessor{
        indexBufferViewIndex,
        0,
        Mesh::ComponentType::UINT32,
        static_cast<uint32_t>(indices.size()),
        Mesh::Type::SCALAR});
    auto primitive = Mesh::Primitive{};
    primitive.attributes[Mesh::Attribute::POSITION] = positionAccessorIndex;
    primitive.attributes[Mesh::Attribute::NORMAL] = normalAccessorIndex;
    primitive.index = indexAccessorIndex;
    Mesh.primitive = std::move(primitive);
    return Mesh;
}
Mesh Geometry::CreateBox()
{
    Mesh Mesh;
    // =================Position=========
    std::array<Vec3, 24> positions = {
        Vec3(-1.0, -1.0, 1.0),
        Vec3(1.0, -1.0, 1.0),
        Vec3(-1.0, 1.0, 1.0),
        Vec3(1.0, 1.0, 1.0),
        Vec3(1.0, -1.0, 1.0),
        Vec3(-1.0, -1.0, 1.0),
        Vec3(1.0, -1.0, -1.0),
        Vec3(-1.0, -1.0, -1.0),
        Vec3(1.0, 1.0, 1.0),
        Vec3(1.0, -1.0, 1.0),
        Vec3(1.0, 1.0, -1.0),
        Vec3(1.0, -1.0, -1.0),
        Vec3(-1.0, 1.0, 1.0),
        Vec3(1.0, 1.0, 1.0),
        Vec3(-1.0, 1.0, -1.0),
        Vec3(1.0, 1.0, -1.0),
        Vec3(-1.0, -1.0, 1.0),
        Vec3(-1.0, 1.0, 1.0),
        Vec3(-1.0, -1.0, -1.0),
        Vec3(-1.0, 1.0, -1.0),
        Vec3(-1.0, -1.0, -1.0),
        Vec3(-1.0, 1.0, -1.0),
        Vec3(1.0, -1.0, -1.0),
        Vec3(1.0, 1.0, -1.0)};
    //=========Normal===============
    std::array<Vec3, 24> normals = {
        Vec3(0, 0, 1),
        Vec3(0, 0, 1),
        Vec3(0, 0, 1),
        Vec3(0, 0, 1),
        Vec3(0, -1, 0),
        Vec3(0, -1, 0),
        Vec3(0, -1, 0),
        Vec3(0, -1, 0),
        Vec3(1, 0, 0),
        Vec3(1, 0, 0),
        Vec3(1, 0, 0),
        Vec3(1, 0, 0),
        Vec3(0, 1, 0),
        Vec3(0, 1, 0),
        Vec3(0, 1, 0),
        Vec3(0, 1, 0),
        Vec3(-1, 0, 0),
        Vec3(-1, 0, 0),
        Vec3(-1, 0, 0),
        Vec3(-1, 0, 0),
        Vec3(0, 0, -1),
        Vec3(0, 0, -1),
        Vec3(0, 0, -1),
        Vec3(0, 0, -1)};
    //========= Indices =============
    std::array<uint16_t, 36> indices = {

        0,
        1,
        2,
        3,
        2,
        1,
        4,
        5,
        6,
        7,
        6,
        5,
        8,
        9,
        10,
        11,
        10,
        9,
        12,
        13,
        14,
        15,
        14,
        13,
        16,
        17,
        18,
        19,
        18,
        17,
        20,
        21,
        22,
        23,
        22,
        21};
    // buffer
    size_t posBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back();
    Mesh.buffers[posBufferIndex].resize(positions.size() * sizeof(Vec3));
    memcpy(Mesh.buffers[posBufferIndex].data(),
           positions.data(),
           Mesh.buffers[posBufferIndex].size());
    size_t normalBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back();
    Mesh.buffers[normalBufferIndex].resize(normals.size() * sizeof(Vec3));
    memcpy(Mesh.buffers[normalBufferIndex].data(), normals.data(), Mesh.buffers[normalBufferIndex].size());
    size_t indexBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back();
    Mesh.buffers[indexBufferIndex].resize(indices.size() * sizeof(uint16_t));
    memcpy(Mesh.buffers[indexBufferIndex].data(), indices.data(), Mesh.buffers[indexBufferIndex].size());
    // buffer view
    size_t posBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back();
    Mesh.bufferViews[posBufferIndex].buffer = posBufferIndex;
    Mesh.bufferViews[posBufferIndex].offset = 0;
    Mesh.bufferViews[posBufferIndex].size = Mesh.buffers[posBufferIndex].size();
    Mesh.bufferViews[posBufferIndex].target = Mesh::Target::Vertex;
    size_t normalBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back();
    Mesh.bufferViews[normalBufferViewIndex].buffer = normalBufferIndex;
    Mesh.bufferViews[normalBufferViewIndex].offset = 0;
    Mesh.bufferViews[normalBufferViewIndex].size = Mesh.buffers[normalBufferIndex].size();
    Mesh.bufferViews[normalBufferViewIndex].target = Mesh::Target::Vertex;
    size_t indexBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back();
    Mesh.bufferViews[indexBufferViewIndex].buffer = indexBufferIndex;
    Mesh.bufferViews[indexBufferViewIndex].offset = 0;
    Mesh.bufferViews[indexBufferViewIndex].size = Mesh.buffers[indexBufferIndex].size();
    Mesh.bufferViews[indexBufferViewIndex].target = Mesh::Target::Index;
    // accessor
    size_t posAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back();
    Mesh.accessors[posAccessorIndex].bufferView = posBufferViewIndex;
    Mesh.accessors[posAccessorIndex].byteOffset = 0;
    Mesh.accessors[posAccessorIndex].componentType = Mesh::ComponentType::FLOAT32;
    Mesh.accessors[posAccessorIndex].count = positions.size();
    Mesh.accessors[posAccessorIndex].type = Mesh::Type::VEC3;
    size_t normalAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back();
    Mesh.accessors[normalAccessorIndex].bufferView = normalBufferViewIndex;
    Mesh.accessors[normalAccessorIndex].byteOffset = 0;
    Mesh.accessors[normalAccessorIndex].componentType = Mesh::ComponentType::FLOAT32;
    Mesh.accessors[normalAccessorIndex].count = normals.size();
    Mesh.accessors[normalAccessorIndex].type = Mesh::Type::VEC3;
    size_t indexAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back();
    Mesh.accessors[indexAccessorIndex].bufferView = indexBufferViewIndex;
    Mesh.accessors[indexAccessorIndex].byteOffset = 0;
    Mesh.accessors[indexAccessorIndex].componentType = Mesh::ComponentType::UINT16;
    Mesh.accessors[indexAccessorIndex].count = indices.size();
    Mesh.accessors[indexAccessorIndex].type = Mesh::Type::SCALAR;
    // primitive
    Mesh.primitive.attributes[Mesh::Attribute::POSITION] = posAccessorIndex;
    Mesh.primitive.attributes[Mesh::Attribute::NORMAL] = normalAccessorIndex;
    Mesh.primitive.index = indexAccessorIndex;
    return Mesh;
}
Mesh Geometry::CreateQuad()
{
    Mesh Mesh;
    static const std::array<Vec2, 4> positions = {
        Vec2(-1.0, -1.0),
        Vec2(1.0, -1.0),
        Vec2(-1.0, 1.0),
        Vec2(1.0, 1.0)};
    static const std::array<Vec2, 4> texCoords = {
        Vec2(0.0, 0.0),
        Vec2(1.0, 0.0),
        Vec2(0.0, 1.0),
        Vec2(1.0, 1.0)};
    static const std::array<uint16_t, 6> indices = {
        0,
        1,
        2,
        1,
        3,
        2};
    size_t posBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back();
    Mesh.buffers[posBufferIndex].resize(positions.size() * sizeof(Vec2));
    memcpy(Mesh.buffers[posBufferIndex].data(), positions.data(), Mesh.buffers[posBufferIndex].size());
    size_t texCoordBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back();
    Mesh.buffers[texCoordBufferIndex].resize(texCoords.size() * sizeof(Vec2));
    memcpy(Mesh.buffers[texCoordBufferIndex].data(), texCoords.data(), Mesh.buffers[texCoordBufferIndex].size());
    size_t indexBufferIndex = Mesh.buffers.size();
    Mesh.buffers.emplace_back();
    Mesh.buffers[indexBufferIndex].resize(indices.size() * sizeof(uint16_t));
    memcpy(Mesh.buffers[indexBufferIndex].data(), indices.data(), Mesh.buffers[indexBufferIndex].size());
    size_t posBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back();
    Mesh.bufferViews[posBufferViewIndex].buffer = posBufferIndex;
    Mesh.bufferViews[posBufferViewIndex].offset = 0;
    Mesh.bufferViews[posBufferViewIndex].size = Mesh.buffers[posBufferIndex].size();
    Mesh.bufferViews[posBufferViewIndex].target = Mesh::Target::Vertex;
    size_t texCoordBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back();
    Mesh.bufferViews[texCoordBufferViewIndex].buffer = texCoordBufferIndex;
    Mesh.bufferViews[texCoordBufferViewIndex].offset = 0;
    Mesh.bufferViews[texCoordBufferViewIndex].size = Mesh.buffers[texCoordBufferIndex].size();
    Mesh.bufferViews[texCoordBufferViewIndex].target = Mesh::Target::Vertex;
    size_t indexBufferViewIndex = Mesh.bufferViews.size();
    Mesh.bufferViews.emplace_back();
    Mesh.bufferViews[indexBufferViewIndex].buffer = indexBufferIndex;
    Mesh.bufferViews[indexBufferViewIndex].offset = 0;
    Mesh.bufferViews[indexBufferViewIndex].size = Mesh.buffers[indexBufferIndex].size();
    Mesh.bufferViews[indexBufferViewIndex].target = Mesh::Target::Index;
    size_t posAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back();
    Mesh.accessors[posAccessorIndex].bufferView = posBufferViewIndex;
    Mesh.accessors[posAccessorIndex].byteOffset = 0;
    Mesh.accessors[posAccessorIndex].componentType = Mesh::ComponentType::FLOAT32;
    Mesh.accessors[posAccessorIndex].count = positions.size();
    Mesh.accessors[posAccessorIndex].type = Mesh::Type::VEC2;
    size_t texCoordAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back();
    Mesh.accessors[texCoordAccessorIndex].bufferView = texCoordBufferViewIndex;
    Mesh.accessors[texCoordAccessorIndex].byteOffset = 0;
    Mesh.accessors[texCoordAccessorIndex].componentType = Mesh::ComponentType::FLOAT32;
    Mesh.accessors[texCoordAccessorIndex].count = texCoords.size();
    Mesh.accessors[texCoordAccessorIndex].type = Mesh::Type::VEC2;
    size_t indexAccessorIndex = Mesh.accessors.size();
    Mesh.accessors.emplace_back();
    Mesh.accessors[indexAccessorIndex].bufferView = indexBufferViewIndex;
    Mesh.accessors[indexAccessorIndex].byteOffset = 0;
    Mesh.accessors[indexAccessorIndex].componentType = Mesh::ComponentType::UINT16;
    Mesh.accessors[indexAccessorIndex].count = indices.size();
    Mesh.accessors[indexAccessorIndex].type = Mesh::Type::SCALAR;
    Mesh.primitive.attributes[Mesh::Attribute::POSITION] = posAccessorIndex;
    Mesh.primitive.attributes[Mesh::Attribute::TEXCOORD] = texCoordAccessorIndex;
    Mesh.primitive.index = indexAccessorIndex;
    return Mesh;
}

} // namespace Aether