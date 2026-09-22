#include <World/System.h>
#include <World/World.h>

#include <cassert>
#include <format>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using Log = std::vector<std::string>;

struct RecorderSystem final : Aether::System
{
    std::string signature;
    std::vector<std::string_view> dependencies;
    Log* log = nullptr;
    Aether::World* world = nullptr;
    bool rebuild = false;
    bool throwOnAttach = false;
    bool eraseSelfOnUpdate = false;
    int* attachCounter = nullptr;
    int* detachCounter = nullptr;

    std::string_view GetSignature() const override { return signature; }
    std::vector<std::string_view> GetDependencies() const override { return dependencies; }

    void OnAttach(Aether::World* attachedWorld) override
    {
        if (attachCounter)
            ++*attachCounter;
        world = attachedWorld;
        if (throwOnAttach)
            throw std::logic_error("attach failure");
    }

    void OnDetach() override
    {
        if (detachCounter)
            ++*detachCounter;
        world = nullptr;
    }

    void OnUpdate(float) override
    {
        Record("update");
        if (eraseSelfOnUpdate)
            world->EraseSystem(this);
    }

    bool NeedRebuildRenderGraph() override
    {
        Record("rebuild");
        return rebuild;
    }

    void OnBuildRenderGraph(Aether::RenderGraph::RenderGraph&) override
    {
        Record("graph");
    }

    void OnUpload(Aether::PendingUploadList&) override
    {
        Record("upload");
    }

    void OnEvent(Aether::Event&) override
    {
        Record("event");
    }

    void OnFrameBegin(std::uint32_t slot) override
    {
        Record(std::format("frame:{}", slot));
    }

    void ExtractRenderData(Aether::Render::RenderFeatureFrame&) override
    {
        Record("extract");
    }

    void Record(std::string_view callback)
    {
        if (log)
            log->emplace_back(signature + ":" + std::string(callback));
    }
};

Aether::Scope<RecorderSystem> MakeSystem(std::string signature,
                                         std::vector<std::string_view> dependencies,
                                         Log* log = nullptr)
{
    auto system = Aether::CreateScope<RecorderSystem>();
    system->signature = std::move(signature);
    system->dependencies = std::move(dependencies);
    system->log = log;
    return system;
}

std::vector<std::string> Names(const Log& log, std::string_view suffix)
{
    std::vector<std::string> names;
    for (const auto& entry : log)
    {
        const auto separator = entry.find(':');
        if (entry.substr(separator + 1) == suffix)
            names.push_back(entry.substr(0, separator));
    }
    return names;
}

template <typename Function>
bool ThrowsLogicError(Function&& function)
{
    try
    {
        function();
    }
    catch (const std::logic_error&)
    {
        return true;
    }
    return false;
}

void AssertOrder(const Log& log, std::string_view suffix, std::initializer_list<std::string_view> expected)
{
    const auto actual = Names(log, suffix);
    assert(actual.size() == expected.size());
    auto expectedIter = expected.begin();
    for (const auto& name : actual)
    {
        assert(name == *expectedIter);
        ++expectedIter;
    }
}
} // namespace

int main()
{
    {
        Aether::World world;
        Log log;
        world.PushSystem(MakeSystem("C", {"B"}, &log));
        world.PushSystem(MakeSystem("B", {"A"}, &log));
        world.PushSystem(MakeSystem("A", {}, &log));
        world.BuildExecutionOrder();

        const auto signatures = world.ExecutionOrderSignatures();
        assert((signatures == std::vector<std::string_view>{"A", "B", "C"}));

        world.OnUpdate(0.0f);
        AssertOrder(log, "update", {"A", "B", "C"});

        log.clear();
        world.NeedRebuildRenderGraph();
        AssertOrder(log, "rebuild", {"A", "B", "C"});

        log.clear();
        Aether::RenderGraph::ResourceArena arena;
        Aether::RenderGraph::ResourceLruPool pool(&arena);
        Aether::RenderGraph::RenderGraph graph(&arena, &pool);
        world.OnBuildRenderGraph(graph);
        AssertOrder(log, "graph", {"A", "B", "C"});

        log.clear();
        Aether::PendingUploadList uploads;
        world.OnUpload(uploads);
        AssertOrder(log, "upload", {"A", "B", "C"});

        log.clear();
        Aether::Event event = std::monostate{};
        world.OnEvent(event);
        AssertOrder(log, "event", {"A", "B", "C"});

        log.clear();
        world.OnFrameBegin(17);
        AssertOrder(log, "frame:17", {"A", "B", "C"});

        log.clear();
        Aether::Render::RenderFeatureFrame frame(1);
        world.ExtractRenderData(frame);
        AssertOrder(log, "extract", {"A", "B", "C"});
    }

    {
        Aether::World world;
        Log log;
        world.PushSystem(MakeSystem("E", {}, &log));
        world.PushSystem(MakeSystem("D", {}, &log));
        world.BuildExecutionOrder();
        world.OnUpdate(0.0f);
        AssertOrder(log, "update", {"E", "D"});
    }

    {
        Aether::World world;
        world.PushSystem(MakeSystem("B", {"A"}));
        assert(ThrowsLogicError([&] { world.BuildExecutionOrder(); }));
        world.PushSystem(MakeSystem("A", {}));
        world.OnUpdate(0.0f);
        assert((world.ExecutionOrderSignatures() == std::vector<std::string_view>{"A", "B"}));
    }

    {
        Aether::World world;
        world.PushSystem(MakeSystem("Same", {}));
        world.PushSystem(MakeSystem("Same", {}));
        assert(ThrowsLogicError([&] { world.BuildExecutionOrder(); }));
    }

    {
        Aether::World world;
        world.PushSystem(MakeSystem("Self", {"Self"}));
        assert(ThrowsLogicError([&] { world.BuildExecutionOrder(); }));
    }

    {
        Aether::World world;
        world.PushSystem(MakeSystem("DuplicateDependency", {"A", "A"}));
        world.PushSystem(MakeSystem("A", {}));
        assert(ThrowsLogicError([&] { world.BuildExecutionOrder(); }));
    }

    {
        Aether::World world;
        world.PushSystem(MakeSystem("A", {"B"}));
        world.PushSystem(MakeSystem("B", {"A"}));
        assert(ThrowsLogicError([&] { world.BuildExecutionOrder(); }));
    }

    {
        Aether::World world;
        world.PushSystem(MakeSystem("A", {"C"}));
        world.PushSystem(MakeSystem("B", {"A"}));
        world.PushSystem(MakeSystem("C", {"B"}));
        assert(ThrowsLogicError([&] { world.BuildExecutionOrder(); }));
    }

    {
        Aether::World world;
        auto a = MakeSystem("A", {});
        auto* aPtr = a.get();
        world.PushSystem(std::move(a));
        world.PushSystem(MakeSystem("B", {"A"}));
        world.BuildExecutionOrder();
        world.EraseSystem(aPtr);
        assert(ThrowsLogicError([&] { world.OnUpdate(0.0f); }));
    }

    {
        Aether::World world;
        auto failing = MakeSystem("Failing", {});
        auto* failingPtr = failing.get();
        int attachCount = 0;
        int detachCount = 0;
        failingPtr->attachCounter = &attachCount;
        failingPtr->detachCounter = &detachCount;
        failingPtr->throwOnAttach = true;
        assert(ThrowsLogicError([&] { world.PushSystem(std::move(failing)); }));
        assert(attachCount == 1);
        assert(detachCount == 1);
        assert(world.ExecutionOrderSignatures().empty());
    }

    {
        Aether::World world;
        Log log;
        auto mutating = MakeSystem("Mutating", {}, &log);
        mutating->eraseSelfOnUpdate = true;
        world.PushSystem(std::move(mutating));
        assert(ThrowsLogicError([&] { world.OnUpdate(0.0f); }));
        log.clear();
        world.OnFrameBegin(3);
        AssertOrder(log, "frame:3", {"Mutating"});
    }

    return 0;
}
