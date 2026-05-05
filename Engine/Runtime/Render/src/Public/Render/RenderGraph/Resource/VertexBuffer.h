#pragma once
#include "Resource.h"
#include <Core/Core.h>
#include <Render/RHI.h>
namespace Aether::RenderGraph
{

struct VertexBufferDesc
{
    size_t size; // in bytes
    bool operator==(const VertexBufferDesc& other) const
    {
        return size == other.size;
    }
};
template <>
struct ResourceDescType<rhi::VertexBuffer>
{
    using Type = VertexBufferDesc;
};
template <>
struct Realize<rhi::VertexBuffer>
{
    Scope<rhi::VertexBuffer> operator()(const VertexBufferDesc& desc)
    {
        return CreateScope<rhi::VertexBuffer>(rhi::VertexBuffer::Create(desc.size));
    }
};
} // namespace Aether::RenderGraph

namespace Aether
{
template <>
struct Hash<RenderGraph::VertexBufferDesc>
{
    std::size_t operator()(const RenderGraph::VertexBufferDesc& value) const
    {
        return std::hash<size_t>()(value.size);
    }
};
} // namespace Aether