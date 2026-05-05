#pragma once
#include <Render/RHI.h>
#include "AccessId.h"
#include "Resource.h"
#include "AccessId.h"

namespace Aether::RenderGraph
{
struct TextureViewDesc
{
    AccessId<rhi::Texture2D> texture;
    rhi::TextureViewDesc desc;
    bool operator==(const TextureViewDesc& other) const
    {
        return texture == other.texture && desc == other.desc;
    }
};
template <>
struct ResourceDescType<rhi::TextureView>
{
    using Type = TextureViewDesc;
};
} // namespace Aether::RenderGraph
namespace Aether
{
template <>
struct Hash<RenderGraph::TextureViewDesc>
{
    std::size_t operator()(const Aether::RenderGraph::TextureViewDesc& value) const
    {
        return Hash<RenderGraph::Handle>{}(value.texture.handle)^ Hash<rhi::TextureViewDesc>{}(value.desc);
    }
};
} // namespace Aether