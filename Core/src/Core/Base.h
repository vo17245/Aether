#pragma once
#include <cstdint>
#include <memory>
#include <ranges>
#include <vector>

// clang-format on
namespace Aether
{

inline constexpr uint64_t Bit(uint8_t bit)
{
    return 1ULL << bit;
}
template <typename... Ts>
inline constexpr uint64_t PackBits(Ts... bits)
{
    return (Bit(bits) | ...);
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
} // namespace Aether
