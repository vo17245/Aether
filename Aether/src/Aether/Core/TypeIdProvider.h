#pragma once
#include <cstdint>
#include <stdint.h>
namespace Aether
{

using TypeId=uint64_t;
template <typename T>
class TypeIdProvider {
public:
    static TypeId ID() {
        static char dummy;
        return (TypeId)&dummy;
    }
};
}//namespace Aether
