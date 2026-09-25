#include <GameFeature/RuntimeFeatureRegistrar.h>
#include <GameFeature/EntityMetadata.h>

#include <cassert>

namespace
{
struct Position { float x = 0.0f; };
struct CounterSystem final : Aether::System
{
    int* counter;
    explicit CounterSystem(int* value) : counter(value) {}
    std::string_view GetSignature() const override { return "sample.counter"; }
    std::vector<std::string_view> GetDependencies() const override { return {}; }
    void OnUpdate(float) override { ++*counter; }
};

}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<Position>
{
    static constexpr std::string_view Type = "sample.position";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const Position& value, SaveContext&) { return Json{{"x", value.x}}; }
    static Result<Position> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != 1 || !json.is_object() || !json.contains("x") || !json["x"].is_number())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "invalid Position"});
        return Position{json["x"].get<float>()};
    }
};
}

namespace
{
Aether::GameFeatures::CompiledFeature MakeFeature(std::string id, std::uint32_t version = 1,
    std::vector<Aether::GameFeatures::FeatureRequirement> dependencies = {}, bool registerPosition = false)
{
    Aether::GameFeatures::CompiledFeature feature;
    feature.descriptor = {std::move(id), version, 1, std::move(dependencies)};
    feature.registerRuntime = [registerPosition](Aether::GameFeatures::RuntimeFeatureRegistrar& registrar) {
        if (registerPosition) return registrar.RegisterComponent<Position>();
        return Aether::GameFeatures::FeatureResult<void>{};
    };
    return feature;
}
}

int main()
{
    using namespace Aether;
    using namespace Aether::GameFeatures;
    ProjectAssets::ProjectManifest manifest;
    manifest.projectId = ProjectAssets::ProjectId::Create();
    manifest.features.push_back({"sample.root", 1});

    auto root = MakeFeature("sample.root", 1, {{"sample.dep", 1}});
    auto dependency = MakeFeature("sample.dep", 1, {}, true);
    auto valid = RuntimeRegistry::Build(manifest, {root, dependency});
    assert(valid);
    assert((*valid)->FeatureOrder().size() == 2);
    assert((*valid)->FeatureOrder()[0] == "sample.dep");
    assert((*valid)->Codecs().Frozen());
    assert((*valid)->FindComponent("sample.position"));

    auto missing = RuntimeRegistry::Build(manifest, {root});
    assert(!missing);
    auto versionMismatch = RuntimeRegistry::Build(manifest, {MakeFeature("sample.root", 2)});
    assert(!versionMismatch);

    auto cycleA = MakeFeature("sample.root", 1, {{"sample.dep", 1}});
    auto cycleB = MakeFeature("sample.dep", 1, {{"sample.root", 1}});
    assert(!RuntimeRegistry::Build(manifest, {cycleA, cycleB}));

    ProjectAssets::ProjectManifest duplicateManifest;
    duplicateManifest.projectId = ProjectAssets::ProjectId::Create();
    duplicateManifest.features = {{"sample.one", 1}, {"sample.two", 1}};
    assert(!RuntimeRegistry::Build(duplicateManifest, {MakeFeature("sample.one", 1, {}, true), MakeFeature("sample.two", 1, {}, true)}));

    Serialization::ComponentCodecRegistry codecs;
    assert((*valid)->Codecs().Frozen());
    auto frozenCodecs = (*valid)->Codecs();
    assert(!frozenCodecs.Register<Position>());

    int updatesA = 0, updatesB = 0;
    SystemRegistration registration{"sample.counter", {}, SystemUpdatePhase::Simulation,
        {WorldRole::Authoring}, [&updatesA](const WorldMountContext&) -> Scope<System> { return CreateScope<CounterSystem>(&updatesA); }};
    std::vector<SystemRegistration> systems{registration};
    World worldA, worldB;
    FeatureMountScope mountA, mountB;
    WorldMountContext context;
    assert(mountA.Mount(worldA, context, systems));
    systems[0].factory = [&updatesB](const WorldMountContext&) -> Scope<System> { return CreateScope<CounterSystem>(&updatesB); };
    assert(mountB.Mount(worldB, context, systems));
    worldA.OnUpdate(0.0f);
    worldB.OnUpdate(0.0f);
    assert(updatesA == 1 && updatesB == 1);
}
