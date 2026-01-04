#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace Aether
{
enum class VertexAttributeFormat : uint8_t
{
    None = 0,
    Float32 = 1,
    Vec2f = 2,
    Vec3f = 3,
    Vec4f = 4,
    UInt32 = 5,
};
struct VertexAttribute
{
    VertexAttributeFormat format;
    uint32_t offset;
    uint32_t bufferIndex;
};
struct VertexBufferLayout
{
    uint32_t stride;
    
};
struct VertexLayout
{
    std::vector<VertexAttribute> attributes;
    std::vector<VertexBufferLayout> buffers;
};

} // namespace Aether