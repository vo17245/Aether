#pragma once

#include <GameFeature/FeatureDescriptor.h>
#include <World/System.h>
#include <World/World.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Aether::ProjectAssets { class CatalogSnapshot; }
namespace Aether::GameFeatures
{
enum class WorldRole { Authoring, Play, AssetPreview };

struct WorldMountContext
{
    WorldRole role = WorldRole::Authoring;
    WorldInstanceId worldInstanceId;
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog;
};

struct SystemRegistration
{
    std::string signature;
    std::vector<std::string> dependencies;
    SystemUpdatePhase phase = SystemUpdatePhase::Simulation;
    std::vector<WorldRole> roles;
    std::function<Scope<System>(const WorldMountContext&)> factory;
};

class FeatureMountScope
{
public:
    FeatureMountScope() = default;
    FeatureMountScope(const FeatureMountScope&) = delete;
    FeatureMountScope& operator=(const FeatureMountScope&) = delete;
    ~FeatureMountScope() { Shutdown(); }

    Serialization::Result<void> Mount(World& world, const WorldMountContext& context,
                                      const std::vector<SystemRegistration>& registrations);
    void Shutdown() noexcept;
    const std::vector<std::string>& Diagnostics() const noexcept { return m_Diagnostics; }
    bool Mounted() const noexcept { return m_World != nullptr; }
private:
    void Rollback() noexcept;
    World* m_World = nullptr;
    std::vector<System*> m_Attached;
    std::vector<std::string> m_Diagnostics;
};
}
