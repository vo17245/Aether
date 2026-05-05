#pragma once
#include "Resource.h"
#include <Core/Core.h>
#include <Render/RHI.h>
namespace Aether::RenderGraph
{

struct RWStructuredBufferDesc
{
    size_t size; // in bytes
    bool operator==(const RWStructuredBufferDesc& other) const
    {
        return size == other.size;
    }
};
template <>
struct ResourceDescType<rhi::RWStructuredBuffer>
{
    using Type = RWStructuredBufferDesc;
};
template <>
struct Realize<rhi::RWStructuredBuffer>
{
    Scope<rhi::RWStructuredBuffer> operator()(const RWStructuredBufferDesc& desc)
    {
        return CreateScope<rhi::RWStructuredBuffer>(rhi::RWStructuredBuffer::Create(desc.size));
    }
};
} // namespace Aether::RenderGraph

namespace Aether
{
template <>
struct Hash<RenderGraph::RWStructuredBufferDesc>
{
    std::size_t operator()(const RenderGraph::RWStructuredBufferDesc& value) const
    {
        return std::hash<size_t>()(value.size);
    }
};
} // namespace Aether