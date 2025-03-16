#pragma once
#include <cstdint>
#include <type_traits>
#include <cstddef>
namespace Aether
{
template <typename T, typename... Ts>
struct TypeIndex;

// 基本情况：如果T是第一个类型，索引为0
template <typename T, typename... Ts>
struct TypeIndex<T, T, Ts...>
{
    static constexpr size_t value = 0;
};

// 递归情况：如果T不是当前类型，则递归计算剩余类型的索引
template <typename T, typename U, typename... Ts>
struct TypeIndex<T, U, Ts...>
{
    static constexpr size_t value = 1 + TypeIndex<T, Ts...>::value;
};

// 如果T不在类型包中，值为-1
template <typename T>
struct TypeIndex<T>
{
    static constexpr size_t value = -1; // 类型不在列表中
};

template<typename T,typename... Ts>
struct IsContainType;
template<typename T>
struct IsContainType<T>
{
    static constexpr bool value=false;
};
template<typename T,typename...  Ts>
struct IsContainType<T,T,Ts...>
{
    static constexpr bool value=true;
};
template<typename T,typename U,typename... Ts>
struct IsContainType<T,U,Ts...>
{
    static constexpr bool value=IsContainType<T,Ts...>::value;
};

template <typename... Ts>
struct TypeArray
{
};
template <typename T, typename U>
struct TypeIndexInArray;
template <typename T, typename... Ts>
struct TypeIndexInArray<T, TypeArray<Ts...>>
{
    static constexpr size_t value = TypeIndex<T, Ts...>::value;
};
template<typename T,typename U>
struct IsArrayContainType;
template<typename T,typename... Ts>
struct IsArrayContainType<T, TypeArray<Ts...>>
{
    static constexpr bool value = IsContainType<T, Ts...>::value;
};

} // namespace Aether