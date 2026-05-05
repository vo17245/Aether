#pragma once
#include "Resource.h"
#include <Core/Core.h>
#include <Render/RHI.h>
namespace Aether::RenderGraph
{

struct IndexBufferDesc
{
    size_t size; // in bytes
    bool operator==(const IndexBufferDesc& other) const
    {
        return size == other.size;
    }
};
template <>
struct ResourceDescType<rhi::IndexBuffer>
{
    using Type = IndexBufferDesc;
};
template <>
struct Realize<rhi::IndexBuffer>
{
    Scope<rhi::IndexBuffer> operator()(const IndexBufferDesc& desc)
    {
        return CreateScope<rhi::IndexBuffer>(rhi::IndexBuffer::Create(desc.size));
    }
};
} // namespace Aether::RenderGraph

namespace Aether
{
template <>
struct Hash<RenderGraph::IndexBufferDesc>
{
    std::size_t operator()(const RenderGraph::IndexBufferDesc& value) const
    {
        return std::hash<size_t>()(value.size);
    }
};
} // namespace Aether