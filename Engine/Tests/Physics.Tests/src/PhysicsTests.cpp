#include "Physics/Physics.h"
#include "doctest/doctest.h"

#include <cmath>
#include <memory>
#include <vector>

using namespace Aether;
using namespace Aether::Physics;

namespace
{
std::unique_ptr<PhysicsEngine> MakeEngine()
{
    auto result = PhysicsEngine::Create();
    REQUIRE(result.has_value());
    return std::move(*result);
}

std::unique_ptr<PhysicsWorld> MakeWorld(PhysicsEngine& engine)
{
    auto result = engine.CreateWorld();
    REQUIRE(result.has_value());
    return std::move(*result);
}
}

TEST_CASE("Physics creates primitive shapes and validates geometry")
{
    auto engine = MakeEngine();
    CHECK(engine->CreateBox({.halfExtent = Vec3f::Constant(1.0f)}).has_value());
    CHECK(engine->CreateSphere({.radius = 0.5f}).has_value());
    auto sphereCapsule = engine->CreateCapsule({.radius = 0.5f, .height = 1.0f});
    REQUIRE(sphereCapsule.has_value());
    CHECK(sphereCapsule->Kind() == ShapeKind::Capsule);
    CHECK_FALSE(engine->CreateSphere({.radius = 0.0f}).has_value());
    CHECK_FALSE(engine->CreateCapsule({.radius = 0.5f, .height = 0.9f}).has_value());
}

TEST_CASE("Physics capsule cast reports fraction, normal and user data")
{
    auto engine = MakeEngine();
    auto world = MakeWorld(*engine);
    auto floor = engine->CreateBox({.halfExtent = Vec3f{5.0f, 0.5f, 5.0f}});
    auto capsule = engine->CreateCapsule({.radius = 0.5f, .height = 2.0f});
    REQUIRE(floor.has_value());
    REQUIRE(capsule.has_value());

    auto body = world->CreateBody({
        .shape = *floor,
        .transform = {.position = Vec3d{0.0, 0.0, 0.0}},
        .layer = 0,
        .userData = 42,
    });
    REQUIRE(body.has_value());

    auto hit = world->CastShapeClosest({
        .shape = *capsule,
        .start = {.position = Vec3d{0.0, 3.0, 0.0}},
        .displacement = Vec3f{0.0f, -4.0f, 0.0f},
    });
    REQUIRE(hit.has_value());
    REQUIRE(hit->has_value());
    CHECK(hit->value().body == *body);
    CHECK(hit->value().bodyUserData == 42);
    CHECK(hit->value().fraction == doctest::Approx(0.375f).epsilon(1e-3));
    CHECK(hit->value().normal.y() == doctest::Approx(1.0f).epsilon(1e-3));
    CHECK(hit->value().pointOnTarget.y() == doctest::Approx(0.5).epsilon(1e-3));
    CHECK_FALSE(hit->value().startedOverlapping);
}

TEST_CASE("Physics filters layers, ignored bodies and stale handles")
{
    auto engine = MakeEngine();
    WorldDesc desc;
    desc.layers = {{0, LayerBit(1)}, {1, LayerBit(0) | LayerBit(1)}};
    auto worldResult = engine->CreateWorld(desc);
    REQUIRE(worldResult.has_value());
    auto world = std::move(*worldResult);
    auto shape = engine->CreateBox({.halfExtent = Vec3f::Constant(1.0f)});
    REQUIRE(shape.has_value());
    auto first = world->CreateBody({.shape = *shape, .layer = 0, .userData = 1});
    auto second = world->CreateBody({.shape = *shape, .transform = {.position = Vec3d{0.0, 3.0, 0.0}}, .layer = 1});
    REQUIRE(first.has_value());
    REQUIRE(second.has_value());

    RayCast ray{.origin = Vec3d{0.0, 5.0, 0.0}, .displacement = Vec3f{0.0f, -10.0f, 0.0f}};
    auto filtered = world->CastRayClosest(ray, {.layers = LayerBit(0)});
    REQUIRE(filtered.has_value());
    REQUIRE(filtered->has_value());
    CHECK(filtered->value().body == *first);

    QueryFilter ignoredFilter{.layers = LayerBit(0) | LayerBit(1), .ignoredBody = *first};
    auto ignored = world->CastRayClosest(ray, ignoredFilter);
    REQUIRE(ignored.has_value());
    REQUIRE(ignored->has_value());
    CHECK(ignored->value().body == *second);

    REQUIRE(world->DestroyBody(*first).has_value());
    CHECK_FALSE(world->IsBodyValid(*first));
    CHECK_FALSE(world->DestroyBody(*first).has_value());
}

TEST_CASE("Physics supports compound child user data and initial overlap")
{
    auto engine = MakeEngine();
    auto world = MakeWorld(*engine);
    auto box = engine->CreateBox({.halfExtent = Vec3f::Constant(0.5f)});
    REQUIRE(box.has_value());
    std::vector<CompoundChild> children = {
        {.shape = *box, .transform = {.position = Vec3f{-1.0f, 0.0f, 0.0f}}, .userData = 7},
        {.shape = *box, .transform = {.position = Vec3f{1.0f, 0.0f, 0.0f}}, .userData = 8},
    };
    auto compound = engine->CreateStaticCompound(children);
    REQUIRE(compound.has_value());
    auto body = world->CreateBody({.shape = *compound, .userData = 99});
    REQUIRE(body.has_value());
    auto capsule = engine->CreateCapsule({.radius = 0.25f, .height = 0.5f});
    REQUIRE(capsule.has_value());
    auto hit = world->CastShapeClosest({
        .shape = *capsule,
        .start = {.position = Vec3d{-1.0, 0.0, 0.0}},
        .displacement = Vec3f{0.0f, 2.0f, 0.0f},
    });
    REQUIRE(hit.has_value());
    REQUIRE(hit->has_value());
    CHECK(hit->value().subShapeUserData == 7);

    auto overlap = world->OverlapShape({.shape = *capsule, .transform = {.position = Vec3d{-1.0, 0.0, 0.0}}});
    REQUIRE(overlap.has_value());
    CHECK_FALSE(overlap->empty());
}

TEST_CASE("Physics preserves double precision at large world coordinates")
{
    auto engine = MakeEngine();
    auto world = MakeWorld(*engine);
    auto box = engine->CreateBox({.halfExtent = Vec3f::Constant(0.5f)});
    REQUIRE(box.has_value());
    const Vec3d base{1.0e9, 0.0, -1.0e9};
    auto body = world->CreateBody({.shape = *box, .transform = {.position = base}});
    REQUIRE(body.has_value());

    auto hit = world->CastRayClosest({
        .origin = base + Vec3d{0.0, 5.0, 0.0},
        .displacement = Vec3f{0.0f, -10.0f, 0.0f},
    });
    REQUIRE(hit.has_value());
    REQUIRE(hit->has_value());
    CHECK(hit->value().body == *body);
    CHECK(hit->value().fraction == doctest::Approx(0.45f).epsilon(1e-4));
    CHECK((hit->value().point - (base + Vec3d{0.0, 0.5, 0.0})).norm() < 1e-6);
}

TEST_CASE("Physics validates layers and keeps runtime alive through facade destruction")
{
    auto badEngine = MakeEngine();
    WorldDesc asymmetric;
    asymmetric.layers = {{0, LayerBit(1)}, {1, 0}};
    CHECK_FALSE(badEngine->CreateWorld(asymmetric).has_value());

    auto engineResult = PhysicsEngine::Create();
    REQUIRE(engineResult.has_value());
    auto engine = std::move(*engineResult);
    auto shapeResult = engine->CreateBox({.halfExtent = Vec3f::Constant(0.5f)});
    REQUIRE(shapeResult.has_value());
    auto worldResult = engine->CreateWorld();
    REQUIRE(worldResult.has_value());
    auto world = std::move(*worldResult);
    auto bodyResult = world->CreateBody({.shape = *shapeResult});
    REQUIRE(bodyResult.has_value());
    engine.reset();

    auto hit = world->CastRayClosest({
        .origin = Vec3d{0.0, 2.0, 0.0},
        .displacement = Vec3f{0.0f, -4.0f, 0.0f},
    });
    REQUIRE(hit.has_value());
    CHECK(hit->has_value());
    world.reset();
    CHECK(PhysicsEngine::Create().has_value());
}
