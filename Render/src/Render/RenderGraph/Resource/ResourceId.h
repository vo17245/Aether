#pragma once
#include "Handle.h"
namespace Aether::RenderGraph
{
    // typed resource id
    template<typename T>
    struct ResourceId
    {
        Handle handle;
    };
}