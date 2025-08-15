#pragma once
#include <tuple>
namespace Aether
{
template <typename T>
constexpr bool always_false_v = false;
template <typename... Ts>
struct TypeArray
{
};
template<typename T>
struct TypeArrayToTuple;
template<typename... Ts>
struct TypeArrayToTuple<TypeArray<Ts...>>
{
    using Type = std::tuple<Ts...>;
};

} // namespace Aether