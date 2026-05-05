#pragma once
#include "Resource.h"
#include <Core/Core.h>
#include <Render/RHI.h>
namespace Aether::RenderGraph
{

struct StagingBufferDesc
{
    size_t size; // in bytes
    bool operator==(const StagingBufferDesc& other) const
    {
        return size == other.size;
    }
};
template <>
struct ResourceDescType<rhi::StagingBuffer>
{
    using Type = StagingBufferDesc;
};
template <>
struct Realize<rhi::StagingBuffer>
{
    Scope<rhi::StagingBuffer> operator()(const StagingBufferDesc& desc)
    {
        return CreateScope<rhi::StagingBuffer>(rhi::StagingBuffer::Create(desc.size));
    }
};
} // namespace Aether::RenderGraph

namespace Aether
{
template <>
struct Hash<RenderGraph::StagingBufferDesc>
{
    std::size_t operator()(const RenderGraph::StagingBufferDesc& value) const
    {
        return std::hash<size_t>()(value.size);
    }
};
} // namespace Aether