#include <Render/DisplaySurfaceRegistry.h>

#include <cassert>

int main()
{
    using namespace Aether;
    using namespace Aether::Render;
    DisplaySurfaceRegistry registry;
    const auto first = registry.Create();
    assert(first.IsValid());
    assert(!registry.Acquire(first));
    assert(registry.Publish(first, {640, 360}));
    auto inFlight = registry.Acquire(first);
    assert(inFlight && inFlight->Token() == first && inFlight->Extent().width == 640);

    const auto second = registry.Replace(first, 7);
    assert(second && second->id == first.id && second->generation == first.generation + 1);
    assert(!registry.Acquire(first));
    assert(!registry.Publish(first, {320, 200}));
    registry.CompleteThrough(7);
    assert(registry.PendingRetirements() == 1);
    inFlight.reset();
    registry.CompleteThrough(7);
    assert(registry.PendingRetirements() == 0);

    assert(registry.Publish(*second, {1280, 720}));
    auto secondLease = registry.Acquire(*second);
    assert(secondLease && secondLease->Extent().height == 720);
    assert(registry.Retire(*second, 9));
    registry.CompleteThrough(8);
    assert(registry.PendingRetirements() == 1);
    secondLease.reset();
    registry.CompleteThrough(9);
    assert(registry.PendingRetirements() == 0);
    assert(!registry.Retire(*second, 9));
}
