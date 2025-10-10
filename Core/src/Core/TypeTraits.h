#pragma once
#include <tuple>
#include "TypeArray.h"
namespace Aether
{
template <typename T>
constexpr bool always_false_v = false;
#ifdef _MSC_VER
template <typename T>
void PrintType()
{
    static_assert(always_false_v<T>, __FUNCSIG__);
}
#else 
template <typename T>
void PrintType()
{
    static_assert(always_false_v<T>, __PRETTY_FUNCTION__);
}
#endif

} // namespace Aether