#pragma once
#include "NotNull.h"
namespace Aether
{
template<class T>
using Borrow = NotNull<T>;   // 若无法引入 GSL，可退化为 T*
}