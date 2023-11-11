#pragma once

#include <stdint.h>
#include <memory>
#include <assert.h>
#define AETHER_NAMESPACE_BEGIN namespace Aether{
#define AETHER_NAMESPACE_END }
AETHER_NAMESPACE_BEGIN
constexpr uint32_t Bit(uint32_t n)
{
	return 1 << n;
}

using ssize_t = int64_t;

template<typename T>
using Ref=std::shared_ptr<T>;
template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T,typename... Ts>
constexpr Ref<T> CreateRef(Ts&&... args)
{
	return std::make_shared<T>(std::forward<Ts>(args)...);
}
template<typename T,typename... Ts>
constexpr Scope<T> CreateScope(Ts&&... args)
{
	return std::make_unique<T>(std::forward<Ts>(args)...);
}
AETHER_NAMESPACE_END




#ifdef _WIN32
	#include "Windows.h" // only aim to remove msvc macro redefine warning
	#ifdef far
		#undef far
	#endif
	#ifdef near
		#undef near
	#endif
#endif

#define AETHER_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }