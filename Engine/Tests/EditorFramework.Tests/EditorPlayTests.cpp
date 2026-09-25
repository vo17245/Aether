#include <EditorFramework/PlayLifecycle.h>

#include <cassert>

namespace
{
struct Counter { int value = 0; };
struct CounterSystem final : Aether::System
{
    Aether::World* world = nullptr;
    int* attached = nullptr;
    int* detached = nullptr;
    CounterSystem(int* attach, int* detach) : attached(attach), detached(detach) {}
    std::string_view GetSignature() const override { return "sample.counter-system"; }
    std::vector<std::string_view> GetDependencies() const override { return {}; }
    void OnAttach(Aether::World* attachedWorld) override { world = attachedWorld; ++*attached; }
    void OnDetach() override { ++*detached; world = nullptr; }
    void OnUpdate(float) override
    {
        for (const auto entity : world->Select<Counter>())
            ++world->GetComponent<Counter>(entity).value;
    }
};
}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<Counter>
{
    static constexpr std::string_view Type = "sample.counter";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const Counter& value, SaveContext&) { return Json{{"value", value.value}}; }
    static Result<Counter> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != 1 || !json.is_object() || !json.contains("value") || !json["value"].is_number_integer())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid counter component"});
        return Counter{json["value"].get<int>()};
    }
};
}

int main()
{
    using namespace Aether;
    using namespace Aether::GameFeatures;
    int attached = 0, detached = 0;
    CompiledFeature feature;
    feature.descriptor = {"sample.play", 1, 1, {}};
    feature.registerRuntime = [&](RuntimeFeatureRegistrar& registrar) -> FeatureResult<void> {
        auto base = RegisterBaseRuntimeComponents(registrar);
        if (!base) return base;
        auto component = registrar.RegisterComponent<Counter>();
        if (!component) return component;
        return registrar.RegisterSystem({"sample.counter-system", {}, SystemUpdatePhase::Simulation, {WorldRole::Play},
            [&](const WorldMountContext&) -> Scope<System> { return CreateScope<CounterSystem>(&attached, &detached); }});
    };
    ProjectAssets::ProjectManifest manifest{ProjectAssets::ProjectId::Create(), {{"sample.play", 1}}};
    auto runtime = RuntimeRegistry::Build(manifest, {feature});
    assert(runtime);
    auto catalog = ProjectAssets::CatalogSnapshot::Create({manifest.projectId, 1, {}}, (*runtime)->AssetTypes());
    assert(catalog);

    World authoring;
    auto entity = authoring.CreateEntity();
    authoring.AddComponent<PersistentEntityIdComponent>(entity, PersistentEntityIdComponent{PersistentEntityId::Create()});
    authoring.AddComponent<Counter>(entity, Counter{5});
    EditorFramework::PlaySessionComponent session;
    auto blocked = EditorFramework::StartPlay(session, authoring, DocumentId::Create(), **runtime, *catalog,
        std::filesystem::temp_directory_path(), {.dirtyAssetDocuments = true});
    assert(!blocked && session.state == EditorFramework::PlayState::Stopped);

    const auto documentId = DocumentId::Create();
    assert(EditorFramework::StartPlay(session, authoring, documentId, **runtime, *catalog,
        std::filesystem::temp_directory_path()));
    assert(attached == 1 && session.playWorld && session.instanceId.IsValid());
    const auto firstWorldId = session.instanceId;
    auto steps = EditorFramework::AdvancePlay(session, 0.1);
    assert(steps && *steps == 4 && session.clock.tickIndex == 4);
    auto playEntity = *session.playWorld->Select<Counter>().begin();
    assert(session.playWorld->GetComponent<Counter>(playEntity).value == 9);
    assert(authoring.GetComponent<Counter>(entity).value == 5);

    assert(EditorFramework::PausePlay(session));
    steps = EditorFramework::AdvancePlay(session, 0.5);
    assert(steps && *steps == 0 && session.clock.tickIndex == 4);
    assert(EditorFramework::StepPlay(session));
    assert(session.clock.tickIndex == 5 && session.playWorld->GetComponent<Counter>(playEntity).value == 10);
    assert(EditorFramework::ResumePlay(session));
    steps = EditorFramework::AdvancePlay(session, 0.005);
    assert(steps && *steps == 0);

    EditorFramework::StopPlay(session);
    assert(session.state == EditorFramework::PlayState::Stopped && detached == 1);
    assert(authoring.GetComponent<Counter>(entity).value == 5);
    assert(EditorFramework::StartPlay(session, authoring, documentId, **runtime, *catalog,
        std::filesystem::temp_directory_path()));
    assert(session.instanceId != firstWorldId && attached == 2);
    EditorFramework::StopPlay(session);
    assert(detached == 2);
}
