#pragma once
#include "Resource.h"
#include "ResourceTypeTraits.h"
#include <Core/Core.h>
#include "AccessId.h"
#include "ResourceCode.h"
namespace Aether::RenderGraph
{

struct TaskBase;
struct VirtualResourceBase
{
    virtual ~VirtualResourceBase() = default;
    std::vector<Borrow<TaskBase>> readers;
    std::vector<Borrow<TaskBase>> writers;
    TaskBase* creator = nullptr;
    ResourceCode code;
    size_t refCount = 0;              // for culling when compile
    int64_t lastAccessTaskIndex = -1; // for resource aliasing when compile
    inline bool Transient() const
    {
        return creator != nullptr;
    }
    std::string tag;
};

template <typename R>
    requires IsResource<R>::value && HasGetResourceCodeImpl<R>::value
struct VirtualResource : public VirtualResourceBase
{
    using ResourceType = R;
    typename ResourceDescType<ResourceType>::Type desc;
    AccessId<ResourceType> id;
    VirtualResource(const typename ResourceDescType<ResourceType>::Type& _desc, const AccessId<ResourceType>& _id) :
        desc(_desc), id(_id)
    {
        code = GetResourceCode<ResourceType>::value;
    }
    VirtualResource()
    {
        code = GetResourceCode<ResourceType>::value;
    }
};

namespace Detail
{
template <typename F, typename T>
struct VirtualResourceVisitorCommanRetType;
template <typename F, typename... Ts>
struct VirtualResourceVisitorCommanRetType<F, TypeArray<Ts...>>
{
    using Type = std::common_type_t<decltype(std::declval<F>()(std::declval<Ts>()))...>;
};
template <typename R, typename F, size_t... Is>
decltype(auto) VisitVirtualResourceImpl(R&& resource, F&& f, std::index_sequence<Is...>)
{
    using Ret = typename VirtualResourceVisitorCommanRetType<F, ResourceTypeArray>::Type;
    using Fn = Ret (*)(VirtualResourceBase&, F&);
    using Tuple = typename TypeArrayToTuple<ResourceTypeArray>::Type;
    static constexpr Fn table[] = {+[](VirtualResourceBase& r, F& f) -> Ret {
#ifdef NDEBUG
        return f(static_cast<VirtualResource<std::tuple_element_t<Is, Tuple>>&>(r));
#else
        return f(dynamic_cast<VirtualResource<std::tuple_element_t<Is, Tuple>>&>(r));
#endif
    }...};
    return table[(size_t)resource.code](resource, f);
}
} // namespace Detail
template <typename R, typename F>
    requires std::is_same_v<VirtualResourceBase, std::decay_t<R>>
decltype(auto) VisitVirtualResource(R&& resource, F&& f)
{
    using Tuple = typename TypeArrayToTuple<ResourceTypeArray>::Type;
    assert(resource.code < ResourceCode::Count && "Invalid resource code");
    return Detail::VisitVirtualResourceImpl(resource, f, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

} // namespace Aether::RenderGraph
