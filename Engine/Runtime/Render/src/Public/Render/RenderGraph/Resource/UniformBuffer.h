#pragma once
#include "Resource.h"
#include <Core/Core.h>
#include <Render/RHI.h>
namespace Aether::RenderGraph
{

struct UniformBufferDesc
{
    size_t size; // in bytes
    bool operator==(const UniformBufferDesc& other) const
    {
        return size == other.size;
    }
};
template <>
struct ResourceDescType<rhi::UniformBuffer>
{
    using Type = UniformBufferDesc;
};
template <>
struct Realize<rhi::UniformBuffer>
{
    Scope<rhi::UniformBuffer> operator()(const UniformBufferDesc& desc)
    {
        return CreateScope<rhi::UniformBuffer>(rhi::UniformBuffer::Create(desc.size));
    }
};
} // namespace Aether::RenderGraph

namespace Aether
{
template <>
struct Hash<RenderGraph::UniformBufferDesc>
{
    std::size_t operator()(const RenderGraph::UniformBufferDesc& value) const
    {
        return std::hash<size_t>()(value.size);
    }
};
} // namespace Aether