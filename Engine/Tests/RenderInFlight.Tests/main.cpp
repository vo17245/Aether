#include <Render/InFlight/InFlightResourceAllocator.h>
#include <Render/RenderGraph/RenderGraph.h>

#include <array>
#include <iostream>
#include <stdexcept>

namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}

int main()
{
    try
    {
    {
        Aether::InFlightResourceAllocator resources(2);
        Check(resources.FrameSlotCount() == 2, "unexpected frame slot count");
        Check(resources.CurrentFrameSlot() == 0, "unexpected initial frame slot");

        bool rejected = false;
        try
        {
            Aether::InFlightResourceAllocator invalid(0);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        Check(rejected, "zero frame slots were accepted");

        rejected = false;
        try
        {
            Aether::InFlightResourceAllocator invalid(Aether::Render::Config::InFlightFrameResourceSlots + 1);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        Check(rejected, "frame slot count above fixed capacity was accepted");
    }

    Aether::RenderGraph::ResourceArena arena;
    const auto baseline = arena.ActiveIdCount();
    std::array<Aether::rhi::UniformBuffer, 2> buffers;
    std::array<Aether::RenderGraph::ResourceId<Aether::rhi::UniformBuffer>, 2> ids;
    for (std::size_t slot = 0; slot < ids.size(); ++slot)
    {
        ids[slot] = arena.Import(&buffers[slot]);
    }
    Check(arena.ActiveIdCount() == baseline + ids.size(), "imports were not registered");

    {
        Aether::RenderGraph::ResourceLruPool pool(&arena);
        Aether::RenderGraph::RenderGraph graph(&arena, &pool);
        const Aether::RenderGraph::UniformBufferDesc desc{.size = 1};
        const auto access = graph.Import(
            "InFlightUniforms", desc, std::span<const Aether::RenderGraph::ResourceId<Aether::rhi::UniformBuffer>>(
                                             ids.data(), ids.size()));
        graph.GetResourceAccessor().SetCurrentFrame(1);
        Check(graph.GetResourceAccessor().GetResource(access) == &buffers[1],
              "frame resource resolved to the wrong slot");
    }

    for (const auto id : ids)
    {
        arena.Destroy(id);
    }
    Check(arena.ActiveIdCount() == baseline, "imports were not released");
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
    return 0;
}
