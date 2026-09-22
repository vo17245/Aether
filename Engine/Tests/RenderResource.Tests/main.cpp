#include <Render/Frame/FrameRetirement.h>
#include <Render/Resource/RenderResourceRegistry.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

namespace
{
void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void TestRegistryGenerationAndRetirement()
{
    using Registry = Aether::Render::RenderResourceRegistry<std::shared_ptr<int>>;
    Registry registry;
    auto resource = std::make_shared<int>(42);
    std::weak_ptr<int> lifetime = resource;
    const auto first = registry.Create(resource);
    resource.reset();
    Check(registry.TryGet(first) && **registry.TryGet(first) == 42, "created resource cannot be resolved");
    Check(registry.MarkUsed(first, 7), "resource use was not recorded");
    Check(registry.Release(first), "resource release failed");
    Check(registry.TryGet(first) == nullptr, "released handle is still resolvable");
    Check(registry.Collect(6) == 0 && !lifetime.expired(), "resource retired before its submission completed");
    Check(registry.Collect(7) == 1 && lifetime.expired(), "resource was not retired at fence completion");

    const auto second = registry.Create(std::make_shared<int>(9));
    Check(second.index == first.index && second.generation != first.generation,
          "reused slot did not change generation");
    Check(registry.TryGet(first) == nullptr, "stale handle resolved to a new resource");

    std::atomic<bool> rejected = false;
    std::thread wrongThread([&] {
        try
        {
            (void)registry.TryGet(second);
        }
        catch (const std::logic_error&)
        {
            rejected = true;
        }
    });
    wrongThread.join();
    Check(rejected, "cross-thread registry access was not rejected");
}

void TestFrameRetirement()
{
    Aether::Render::FrameRetirement retirement;
    auto object = std::make_shared<int>(1);
    std::weak_ptr<int> lifetime = object;
    retirement.Retain(11, object);
    object.reset();
    Check(retirement.Collect(10) == 0 && !lifetime.expired(), "frame object retired before fence completion");
    Check(retirement.Collect(11) == 1 && lifetime.expired(), "frame object was not released at completion");
}
} // namespace

int main()
{
    try
    {
        TestRegistryGenerationAndRetirement();
        TestFrameRetirement();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
    return 0;
}
