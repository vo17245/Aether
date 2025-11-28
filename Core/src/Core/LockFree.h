#pragma once
#include <concurrentqueue/moodycamel/concurrentqueue.h>

namespace Aether::LockFree
{
template <typename T>
using Queue = moodycamel::ConcurrentQueue<T>;
}