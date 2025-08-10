#pragma once
#include <Core/Core.h>
namespace Aether::RenderGraph
{
    struct VirtualResourceBase;
    struct TaskBase
    {
        virtual ~TaskBase() = default;
        std::vector<Borrow<VirtualResourceBase>> reads;
        std::vector<Borrow<VirtualResourceBase>> writes;
        std::vector<Borrow<VirtualResourceBase>> creates;

    };
}