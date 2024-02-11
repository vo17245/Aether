#pragma once
#include <cstdint>
template <typename T>
class TypeIdProvider {
public:
    static uint64_t ID() {
        static char dummy;
        return (uint64_t)&dummy;
    }
};