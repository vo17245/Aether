#include <GameFeature/EntityMetadata.h>
#include <GameFeature/WorldMount.h>
#include <World/System.h>
#include <World/World.h>
#include <World/Serialization/WorldArchive.h>

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
struct PhaseCounter final : Aether::System
{
    std::string signature;
    Aether::SystemUpdatePhase phase = Aether::SystemUpdatePhase::Simulation;
    std::vector<std::string_view> dependencies;
    int* count = nullptr;
    int* detached = nullptr;
    std::string nameOnDetach;
    std::vector<std::string>* detachOrder = nullptr;
    bool failAttach = false;
    std::string_view GetSignature() const override { return signature; }
    std::vector<std::string_view> GetDependencies() const override { return dependencies; }
    Aether::SystemUpdatePhase GetUpdatePhase() const noexcept override { return phase; }
    void OnUpdate(float) override { if (count) ++*count; }
    void OnDetach() override
    {
        if (detached) ++*detached;
        if (detachOrder) detachOrder->push_back(nameOnDetach);
    }
    void OnAttach(Aether::World*) override { if (failAttach) throw std::runtime_error("attach failure"); }
};

struct EntityRef { Aether::EntityId value = entt::null; };
}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<EntityRef>
{
    static constexpr std::string_view Type = "test.entity-ref";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const EntityRef& component, SaveContext& context)
    {
        return context.EncodeEntity(component.value);
    }
    static Result<EntityRef> Deserialize(const Json&, std::uint32_t, LoadContext&)
    {
        return EntityRef{};
    }
};
}

int main()
{
    {
        Aether::World world;
        int simulation = 0, presentation = 0;
        auto sim = Aether::CreateScope<PhaseCounter>();
        sim->signature = "simulation";
        sim->count = &simulation;
        world.PushSystem(std::move(sim));
        auto view = Aether::CreateScope<PhaseCounter>();
        view->signature = "presentation";
        view->phase = Aether::SystemUpdatePhase::Presentation;
        view->count = &presentation;
        world.PushSystem(std::move(view));
        world.OnUpdatePhase(Aether::SystemUpdatePhase::Presentation, 1.0f);
        assert(simulation == 0 && presentation == 1);
        world.OnUpdatePhase(Aether::SystemUpdatePhase::Simulation, 1.0f);
        assert(simulation == 1 && presentation == 1);
    }
    {
        Aether::World world;
        auto late = Aether::CreateScope<PhaseCounter>();
        late->signature = "late";
        late->phase = Aether::SystemUpdatePhase::Presentation;
        world.PushSystem(std::move(late));
        auto early = Aether::CreateScope<PhaseCounter>();
        early->signature = "early";
        early->phase = Aether::SystemUpdatePhase::EditorTools;
        early->dependencies = {"late"};
        world.PushSystem(std::move(early));
        bool rejected = false;
        try { world.OnUpdatePhase(Aether::SystemUpdatePhase::EditorTools, 0.0f); }
        catch (const std::logic_error&) { rejected = true; }
        assert(rejected);
    }
    {
        Aether::World world;
        std::vector<std::string> detachOrder;
        Aether::GameFeatures::FeatureMountScope mount;
        std::vector<Aether::GameFeatures::SystemRegistration> registrations;
        for (int i = 0; i < 2; ++i)
        {
            const auto signature = i == 0 ? "first" : "second";
            registrations.push_back({signature, {}, Aether::SystemUpdatePhase::Simulation,
                {Aether::GameFeatures::WorldRole::Authoring}, [&, signature](const auto&) -> Aether::Scope<Aether::System> {
                    if (signature == "second") throw std::runtime_error("factory failure");
                    auto system = Aether::CreateScope<PhaseCounter>();
                    system->signature = signature;
                    system->nameOnDetach = signature;
                    system->detachOrder = &detachOrder;
                    return system;
                }});
        }
        Aether::GameFeatures::WorldMountContext context;
        context.role = Aether::GameFeatures::WorldRole::Authoring;
        auto result = mount.Mount(world, context, registrations);
        assert(!result && !mount.Mounted());
        assert(world.ExecutionOrderSignatures().empty());
        assert((detachOrder == std::vector<std::string>{"first"}));
    }
    {
        Aether::Serialization::ComponentCodecRegistry codecs;
        assert(Aether::GameFeatures::RegisterEntityMetadata(codecs));
        assert(codecs.Register<EntityRef>());
        Aether::World world;
        const auto service = world.CreateEntity();
        world.AddComponent<Aether::Serialization::ExcludeFromArchiveComponent>(service);
        const auto authoredEmpty = world.CreateEntity();
        Aether::Serialization::SaveContext context;
        auto archive = world.Serialize(codecs, context);
        assert(archive && (*archive)["entities"].size() == 1);
        assert((*archive)["entities"][0]["components"].empty());

        const auto owner = world.CreateEntity();
        world.AddComponent<EntityRef>(owner, EntityRef{service});
        Aether::Serialization::SaveContext refContext;
        auto withDanglingRef = world.Serialize(codecs, refContext);
        assert(!withDanglingRef);
        (void)authoredEmpty;
    }
}
