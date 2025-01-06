#include "Geometry.h"
#include <cstdint>
namespace Aether {
Grid Geometry::CreateSphere(float radius, uint32_t segments)
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

    Grid Grid;
    uint32_t positionBufferIndex = Grid.buffers.size();
    Grid.buffers.emplace_back(
        Grid::Buffer(
            (uint8_t*)positions.data(),
            (uint8_t*)(positions.data()) + positions.size() * sizeof(Vec3)));
    uint32_t normalBufferIndex = Grid.buffers.size();
    Grid.buffers.emplace_back(
        Grid::Buffer(
            (uint8_t*)normals.data(),
            (uint8_t*)(normals.data()) + normals.size() * sizeof(Vec3)));
    uint32_t indexBufferIndex = Grid.buffers.size();
    Grid.buffers.emplace_back(Grid::Buffer(
        (uint8_t*)indices.data(),
        (uint8_t*)(indices.data()) + indices.size() * sizeof(uint32_t)));
    uint32_t psitionBufferViewIndex = Grid.bufferViews.size();
    Grid.bufferViews.emplace_back(Grid::BufferView{
        0,
        static_cast<uint32_t>(positions.size() * sizeof(Vec3)),
        positionBufferIndex,
        Grid::Target::Vertex});
    uint32_t normalBufferViewIndex = Grid.bufferViews.size();
    Grid.bufferViews.emplace_back(Grid::BufferView{
        0,
        static_cast<uint32_t>(normals.size() * sizeof(Vec3)),
        normalBufferIndex,
        Grid::Target::Vertex});
    uint32_t indexBufferViewIndex = Grid.bufferViews.size();
    Grid.bufferViews.emplace_back(Grid::BufferView{
        0,
        static_cast<uint32_t>(indices.size() * sizeof(uint32_t)),
        indexBufferIndex,
        Grid::Target::Index});
    uint32_t positionAccessorIndex = Grid.accessors.size();
    Grid.accessors.emplace_back(Grid::Accessor{
        psitionBufferViewIndex,
        0,
        Grid::ComponentType::FLOAT32,
        8,
        Grid::Type::VEC3,
        static_cast<uint32_t>(positions.size())});
    uint32_t normalAccessorIndex = Grid.accessors.size();
    Grid.accessors.emplace_back(Grid::Accessor{
        normalBufferViewIndex,
        0,
        Grid::ComponentType::FLOAT32,
        8,
        Grid::Type::VEC3,
        static_cast<uint32_t>(normals.size())});
    uint32_t indexAccessorIndex = Grid.accessors.size();
    Grid.accessors.emplace_back(Grid::Accessor{
        indexBufferViewIndex,
        0,
        Grid::ComponentType::UINT32,
        static_cast<uint32_t>(indices.size()),
        Grid::Type::SCALAR,
        0});
    auto primitive = Grid::Primitive{};
    primitive.attributes[Grid::Primitive::Attribute::POSITION] = positionAccessorIndex;
    primitive.attributes[Grid::Primitive::Attribute::NORMAL] = normalAccessorIndex;
    primitive.index = indexAccessorIndex;
    Grid.primitive = std::move(primitive);
    return Grid;
}
Grid Geometry::CreateBox()
{
    Grid Grid;
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
    size_t posBufferIndex = Grid.buffers.size();
    Grid.buffers.emplace_back();
    Grid.buffers[posBufferIndex].resize(positions.size() * sizeof(Vec3));
    memcpy(Grid.buffers[posBufferIndex].data(),
           positions.data(),
           Grid.buffers[posBufferIndex].size());
    size_t normalBufferIndex = Grid.buffers.size();
    Grid.buffers.emplace_back();
    Grid.buffers[normalBufferIndex].resize(normals.size() * sizeof(Vec3));
    memcpy(Grid.buffers[normalBufferIndex].data(), normals.data(), Grid.buffers[normalBufferIndex].size());
    size_t indexBufferIndex = Grid.buffers.size();
    Grid.buffers.emplace_back();
    Grid.buffers[indexBufferIndex].resize(indices.size() * sizeof(uint16_t));
    memcpy(Grid.buffers[indexBufferIndex].data(), indices.data(), Grid.buffers[indexBufferIndex].size());
    // buffer view
    size_t posBufferViewIndex = Grid.bufferViews.size();
    Grid.bufferViews.emplace_back();
    Grid.bufferViews[posBufferIndex].buffer = posBufferIndex;
    Grid.bufferViews[posBufferIndex].offset = 0;
    Grid.bufferViews[posBufferIndex].size = Grid.buffers[posBufferIndex].size();
    Grid.bufferViews[posBufferIndex].target = Grid::Target::Vertex;
    size_t normalBufferViewIndex = Grid.bufferViews.size();
    Grid.bufferViews.emplace_back();
    Grid.bufferViews[normalBufferViewIndex].buffer = normalBufferIndex;
    Grid.bufferViews[normalBufferViewIndex].offset = 0;
    Grid.bufferViews[normalBufferViewIndex].size = Grid.buffers[normalBufferIndex].size();
    Grid.bufferViews[normalBufferViewIndex].target = Grid::Target::Vertex;
    size_t indexBufferViewIndex = Grid.bufferViews.size();
    Grid.bufferViews.emplace_back();
    Grid.bufferViews[indexBufferViewIndex].buffer = indexBufferIndex;
    Grid.bufferViews[indexBufferViewIndex].offset = 0;
    Grid.bufferViews[indexBufferViewIndex].size = Grid.buffers[indexBufferIndex].size();
    Grid.bufferViews[indexBufferViewIndex].target = Grid::Target::Index;
    // accessor
    size_t posAccessorIndex = Grid.accessors.size();
    Grid.accessors.emplace_back();
    Grid.accessors[posAccessorIndex].bufferView = posBufferViewIndex;
    Grid.accessors[posAccessorIndex].byteOffset = 0;
    Grid.accessors[posAccessorIndex].componentType = Grid::ComponentType::FLOAT32;
    Grid.accessors[posAccessorIndex].count = positions.size();
    Grid.accessors[posAccessorIndex].type = Grid::Type::VEC3;
    Grid.accessors[posAccessorIndex].byteStride = sizeof(Vec3);
    size_t normalAccessorIndex = Grid.accessors.size();
    Grid.accessors.emplace_back();
    Grid.accessors[normalAccessorIndex].bufferView = normalBufferViewIndex;
    Grid.accessors[normalAccessorIndex].byteOffset = 0;
    Grid.accessors[normalAccessorIndex].componentType = Grid::ComponentType::FLOAT32;
    Grid.accessors[normalAccessorIndex].count = normals.size();
    Grid.accessors[normalAccessorIndex].type = Grid::Type::VEC3;
    Grid.accessors[normalAccessorIndex].byteStride = sizeof(Vec3);
    size_t indexAccessorIndex = Grid.accessors.size();
    Grid.accessors.emplace_back();
    Grid.accessors[indexAccessorIndex].bufferView = indexBufferViewIndex;
    Grid.accessors[indexAccessorIndex].byteOffset = 0;
    Grid.accessors[indexAccessorIndex].componentType = Grid::ComponentType::UINT16;
    Grid.accessors[indexAccessorIndex].count = indices.size();
    Grid.accessors[indexAccessorIndex].type = Grid::Type::SCALAR;
    Grid.accessors[indexAccessorIndex].byteStride = sizeof(uint16_t);
    // primitive
    Grid.primitive.attributes[Grid::Primitive::Attribute::POSITION] = posAccessorIndex;
    Grid.primitive.attributes[Grid::Primitive::Attribute::NORMAL] = normalAccessorIndex;
    Grid.primitive.index = indexAccessorIndex;
    return Grid;
}

} // namespace Kamui