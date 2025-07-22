#pragma once
#include "RenderTask.h"
namespace Aether::TaskGraph
{
    struct RenderTaskArray
    {
        std::vector<Borrow<RenderTaskBase>> tasks;
        RenderPassDesc renderPassDesc;
    };
}