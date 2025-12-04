#pragma once
#include <cstdint>
namespace Aether
{
    struct InitParams
    {
        bool enableGlobalThreadPool;
        uint16_t globalThreadPoolThreadCount;
    };
}