#pragma once
#include <type_traits>
#include <memory>

#include "Resource.h"
namespace Aether::RenderGraph
{
namespace Detail
{


template <typename T, typename = void>
struct HasDescType
{
    static constexpr bool value = false;
};
template <typename T>
struct HasDescType<T, std::void_t<typename ResourceDescType<T>::Type>>
{
    static constexpr bool value = true;
};
template <typename T, typename = void>
struct HasRealizeSpec
{
    static constexpr bool value = false;
};
template <typename T>
struct HasRealizeSpec<T, std::void_t<Realize<T>>>
{
    static constexpr bool value = true;
};
template <typename T,typename=void>
struct Realize_HasOperator
{
    static constexpr bool value = false;
};
template <typename T>
struct Realize_HasOperator<Realize<T>,std::void_t<decltype(std::declval<Realize<T>>().operator()(
            std::declval<const typename ResourceDescType<T>::Type&>()))>>
{
    static constexpr bool value = std::is_same_v<
        decltype(std::declval<Realize<T>>().operator()(
            std::declval<const typename ResourceDescType<T>::Type&>())),
        std::unique_ptr<T>>;
};
template <typename T>
struct HasRealizeImpl
{
    static constexpr bool value = Detail::HasRealizeSpec<T>::value && Detail::Realize_HasOperator<Realize<T>>::value;
};
} // namespace Detail
template <typename T>
struct IsResource
{
    static constexpr bool value = Detail::HasDescType<T>::value && Detail::HasRealizeImpl<T>::value;
};




} // namespace Aether::RenderGraph