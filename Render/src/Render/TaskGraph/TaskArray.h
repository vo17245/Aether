#pragma once
#include "RenderTask.h"
namespace Aether::TaskGraph
{
    struct RenderTaskArray
    {
        std::vector<RenderTaskBase> tasks;
        RenderPassDesc renderPassDesc;
    };
}