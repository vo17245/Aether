#pragma once
#include "Resource.h"
#include "ResourceTypeTraits.h"
#include <Core/Core.h>
#include "AccessId.h"
namespace Aether::RenderGraph
{
enum class ResourceCode : uint32_t
{
    Texture = 0,
    TextureView,
    VertexBuffer,
    IndexBuffer,
    StagingBuffer,
    UniformBuffer,
    RWStructuredBuffer,
    Count,
};
template <typename ResourceType>
struct GetResourceCode;
template <typename ResourceType, typename = void>
struct HasGetResourceCodeImpl
{
    static constexpr bool value = false;
};
template <typename ResourceType>
struct HasGetResourceCodeImpl<ResourceType, std::void_t<decltype(GetResourceCode<ResourceType>::value)>>
{
    static constexpr bool value =
        std::is_same_v<std::decay_t<decltype(GetResourceCode<ResourceType>::value)>, ResourceCode>;
};
} // namespace Aether::RenderGraph
// specialize GetResourceCode for each resource type
#include <Render/RHI.h>
namespace Aether::RenderGraph
{
template <>
struct GetResourceCode<rhi::Texture2D>
{
    static constexpr ResourceCode value = ResourceCode::Texture;
};
template <>
struct GetResourceCode<rhi::TextureView>
{
    static constexpr ResourceCode value = ResourceCode::TextureView;
};

template <>
struct GetResourceCode<rhi::VertexBuffer>
{
    static constexpr ResourceCode value = ResourceCode::VertexBuffer;
};
template <>
struct GetResourceCode<rhi::IndexBuffer>
{
    static constexpr ResourceCode value = ResourceCode::IndexBuffer;
};
template <>
struct GetResourceCode<rhi::StagingBuffer>
{
    static constexpr ResourceCode value = ResourceCode::StagingBuffer;
};
template <>
struct GetResourceCode<rhi::UniformBuffer>
{
    static constexpr ResourceCode value = ResourceCode::UniformBuffer;
};
template <>
struct GetResourceCode<rhi::RWStructuredBuffer>
{
    static constexpr ResourceCode value = ResourceCode::RWStructuredBuffer;
};

using ResourceTypeArray = TypeArray<rhi::Texture2D, rhi::TextureView, rhi::VertexBuffer, rhi::IndexBuffer,
                                    rhi::StagingBuffer, rhi::UniformBuffer, rhi::RWStructuredBuffer>;
template <template <typename> typename ContainerType, typename T>
struct BuildResourcesContainer;
template <template <typename> typename ContainerType, typename... Ts>
struct BuildResourcesContainer<ContainerType, TypeArray<Ts...>>
{
    using Type = std::tuple<ContainerType<Ts>...>;
};
namespace Detail
{
template <typename F, typename Tuple, std::size_t... Is>
decltype(auto) VisitResourcesContainerImpl(Tuple& container, F&& f, size_t index, std::index_sequence<Is...>)
{
    using Ret = std::common_type_t<decltype(f(std::get<Is>(container)))...>;
    using Fn = Ret (*)(Tuple&, F&);
    static constexpr F table[] = {+[](Tuple& t, F& f) -> Ret { return f(std::get<Is>(t)); }...};
    return table[index](container, f);
}
} // namespace Detail
template <typename ResourceContainerType, typename F>
decltype(auto) VisitResourcesContainer(ResourceContainerType& container, F&& f, size_t index)
{
    assert(index < (int)ResourceCode::Count);
    return Detail::VisitResourcesContainerImpl(container, std::forward<F>(f), index,
                                               std::make_index_sequence<std::tuple_size_v<ResourceContainerType>>{});
}

} // namespace Aether::RenderGraph