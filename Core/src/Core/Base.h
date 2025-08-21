#pragma once
#include <cstdint>
#include <memory>
#include <ranges>
#include <vector>
#include <tuple>
// clang-format on
namespace Aether
{

inline constexpr uint64_t Bit(uint8_t bit)
{
    return 1ULL << bit;
}
template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
Ref<T> CreateRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
Scope<T> CreateScope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename... Ts>
struct AreAllScopedEnum;
template<typename T>
struct AreAllScopedEnum<T>
{
    static constexpr bool value = std::is_scoped_enum_v<T>;
};
template<typename T, typename U,typename... Ts>
struct AreAllScopedEnum<T,U,Ts...>
{
    static constexpr bool value = std::is_scoped_enum_v<T> && AreAllScopedEnum<U,Ts...>::value;
};

template<typename... Ts>
requires AreAllScopedEnum<Ts...>::value
inline constexpr auto PackFlags(const Ts&... flags)
{
    return (static_cast<std::underlying_type_t<Ts>>(flags) | ...);
}
template<typename T,typename U>
requires std::is_scoped_enum_v<U>
inline constexpr bool IsFlagSet(T flags, U flag)
{
    return (flags & static_cast<std::underlying_type_t<U>>(flag)) ;
}
namespace Detail
{
template<typename T,typename F, size_t... Is>
inline constexpr void ForEachInTupleImpl(T&& tuple, F&& f, std::index_sequence<Is...>)
{
    (f(std::get<Is>(std::forward<T>(tuple))) , ...);
}
}
template<typename Tuple,typename F>
inline constexpr void ForEachInTuple(Tuple&& tuple,F&& f)
{
    using Indices = std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>;
    Detail::ForEachInTupleImpl(std::forward<Tuple>(tuple), std::forward<F>(f), Indices{});
}
} // namespace Aether
