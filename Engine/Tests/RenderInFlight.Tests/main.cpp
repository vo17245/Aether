#include <Render/InFlight/InFlightResourceAllocator.h>
#include <Render/RenderGraph/RenderGraph.h>

#include <cassert>
#include <array>
#include <stdexcept>

int main()
{
    {
        Aether::InFlightResourceAllocator resources(2);
        assert(resources.FrameSlotCount() == 2);
        assert(resources.CurrentFrameSlot() == 0);

        bool rejected = false;
        try
        {
            Aether::InFlightResourceAllocator invalid(0);
        }
        catch (const std::invalid_argument&)
        {
            rejected = true;
        }
        assert(rejected);
    }

    Aether::RenderGraph::ResourceArena arena;
    const auto baseline = arena.ActiveIdCount();
    std::array<Aether::rhi::UniformBuffer, 2> buffers;
    std::array<Aether::RenderGraph::ResourceId<Aether::rhi::UniformBuffer>, 2> ids;
    for (std::size_t slot = 0; slot < ids.size(); ++slot)
    {
        ids[slot] = arena.Import(&buffers[slot]);
    }
    assert(arena.ActiveIdCount() == baseline + ids.size());

    {
        Aether::RenderGraph::ResourceLruPool pool(&arena);
        Aether::RenderGraph::RenderGraph graph(&arena, &pool);
        const Aether::RenderGraph::UniformBufferDesc desc{.size = 1};
        const auto access = graph.Import(
            "InFlightUniforms", desc, std::span<const Aether::RenderGraph::ResourceId<Aether::rhi::UniformBuffer>>(
                                             ids.data(), ids.size()));
        graph.GetResourceAccessor().SetCurrentFrame(1);
        assert(graph.GetResourceAccessor().GetResource(access) == &buffers[1]);
    }

    for (const auto id : ids)
    {
        arena.Destroy(id);
    }
    assert(arena.ActiveIdCount() == baseline);
    return 0;
}
