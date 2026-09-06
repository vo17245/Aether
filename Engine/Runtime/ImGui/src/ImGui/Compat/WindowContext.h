#pragma once
#include <Render/RenderGraph/RenderGraph.h>
#include <array>
#include <memory>

namespace Aether::ImGuiApi
{
struct WindowContext
{
    struct Frame
    {
        RenderGraph::ResourceArena arena;
        RenderGraph::ResourceLruPool pool{&arena};
        RenderGraph::RenderGraph graph{&arena, &pool};
    };
    // Replace a slot only after Window has waited for that slot's submission fence.
    std::array<std::unique_ptr<Frame>, Render::Config::InFlightFrameResourceSlots> frames;
};
}
