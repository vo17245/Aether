#include <EditorImGui/CorePanels.h>

#include <cassert>

namespace
{
struct Sink final : Aether::EditorImGui::UiIntentSink
{
    void Submit(std::string, Aether::Json) override { ++submitted; }
    int submitted = 0;
};
}

int main()
{
    using namespace Aether::EditorImGui;
    UiSystemRegistry systems;
    assert(RegisterCorePanels(systems));
    assert(systems.Size() == 6);
    assert(systems.Contains("core.documents"));
    assert(systems.Contains("core.hierarchy"));
    assert(systems.Contains("core.inspector"));
    assert(systems.Contains("core.assets"));
    assert(systems.Contains("core.toolbar"));
    assert(systems.Contains("core.viewport"));

    UiSystemRegistry subset;
    constexpr CorePanel selected[]{CorePanel::Hierarchy, CorePanel::Inspector};
    assert(RegisterCorePanels(subset, selected));
    assert(subset.Size() == 2 && subset.Contains("core.hierarchy") && subset.Contains("core.inspector"));
    assert(!subset.Contains("core.assets"));
    assert(!RegisterCorePanels(subset, selected));
    assert(subset.Size() == 2);
    assert(subset.Unregister("core.hierarchy"));
    assert(!subset.Unregister("core.hierarchy"));

    bool readOnlyCallbackRan = false;
    UiSystemRegistry callbackOnly;
    assert(callbackOnly.Register("sample.read-only", [&](const Aether::World&, UiIntentSink&) {
        readOnlyCallbackRan = true;
    }));
    Sink sink;
    Aether::World world;
    callbackOnly.Draw(world, sink);
    assert(readOnlyCallbackRan && sink.submitted == 0);
}
