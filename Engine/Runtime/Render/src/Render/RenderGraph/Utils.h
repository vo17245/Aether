#pragma once
#include <type_traits>
namespace Aether::RenderGraph
{
namespace Detail
{
template <typename ArrayType, size_t index, typename Arg, typename... Args>
inline void SetArrayImpl(ArrayType& array, Arg&& arg, Args&&... args)
{
    if constexpr (index < ArrayType::size())
    {
        array[index] = std::forward<Arg>(arg);
        SetArrayImpl<ArrayType, index + 1>(array, std::forward<Args>(args)...);
    }
    else { static_assert(index == ArrayType::size(), "Array size mismatch"); }
}

}; // namespace Detail
template <typename ArrayType, typename... Args>
inline void SetArray(ArrayType& array, Args&&... args)
{
    SetArrayImpl<ArrayType, 0>(array, std::forward<Args>(args)...);
}
} // namespace Aether::RenderGraph