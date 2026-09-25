#include <GameFeature/RuntimeFeatureRegistrar.h>
#include <GameFeature/EntityMetadata.h>

#include <algorithm>
#include <set>
#include <unordered_set>

namespace Aether::GameFeatures
{
RuntimeFeatureRegistrar::RuntimeFeatureRegistrar(FeatureDescriptor feature, Serialization::ComponentCodecRegistry& codecs,
    ProjectAssets::AssetTypeRegistry& assets, std::unordered_map<std::string, RuntimeComponentDescriptor>& components,
    std::vector<SystemRegistration>& systems)
    : m_Feature(std::move(feature)), m_Codecs(codecs), m_Assets(assets), m_Components(components), m_Systems(systems)
{}

ProjectAssets::Result<void> RuntimeFeatureRegistrar::RegisterAssetType(ProjectAssets::AssetTypeDescriptor descriptor)
{
    if (m_Frozen) return std::unexpected(ProjectAssets::Error{ProjectAssets::ErrorCode::Frozen, "runtime registrar is frozen"});
    if (descriptor.ownerFeature != m_Feature.id)
        return std::unexpected(ProjectAssets::Error{ProjectAssets::ErrorCode::InvalidArgument, "asset type owner differs from registering Feature", descriptor.id});
    return m_Assets.Register(std::move(descriptor));
}

FeatureResult<void> RuntimeFeatureRegistrar::RegisterSystem(SystemRegistration registration)
{
    if (m_Frozen) return std::unexpected(FeatureError{m_Feature.id, "runtime registrar is frozen"});
    if (!ProjectAssets::IsStableIdentifier(registration.signature) || !registration.factory)
        return std::unexpected(FeatureError{m_Feature.id, "system registration requires a stable signature and factory"});
    for (const auto& existing : m_Systems)
        if (existing.signature == registration.signature)
            return std::unexpected(FeatureError{m_Feature.id, "system signature is already registered: " + registration.signature});
    for (const auto& dependency : registration.dependencies)
        if (!ProjectAssets::IsStableIdentifier(dependency))
            return std::unexpected(FeatureError{m_Feature.id, "system dependency signature is invalid: " + dependency});
    m_Systems.push_back(std::move(registration));
    return {};
}

FeatureResult<std::shared_ptr<const RuntimeRegistry>> RuntimeRegistry::Build(
    const ProjectAssets::ProjectManifest& manifest, std::vector<CompiledFeature> compiled)
{
    std::unordered_map<std::string, const CompiledFeature*> entries;
    for (const auto& feature : compiled)
    {
        const auto& descriptor = feature.descriptor;
        if (!ProjectAssets::IsStableIdentifier(descriptor.id) || descriptor.version == 0 || descriptor.apiVersion != 1 || !feature.registerRuntime)
            return std::unexpected(FeatureError{descriptor.id, "compiled feature descriptor or registration function is invalid"});
        if (!entries.emplace(descriptor.id, &feature).second)
            return std::unexpected(FeatureError{descriptor.id, "duplicate compiled Feature ID"});
    }

    std::unordered_map<std::string, std::uint32_t> requested;
    for (const auto& requirement : manifest.features)
    {
        if (!ProjectAssets::IsStableIdentifier(requirement.id) || requirement.version == 0)
            return std::unexpected(FeatureError{requirement.id, "manifest Feature requirement is invalid"});
        if (!requested.emplace(requirement.id, requirement.version).second)
            return std::unexpected(FeatureError{requirement.id, "duplicate Feature requirement in project manifest"});
    }

    std::unordered_map<std::string, std::uint8_t> visitState;
    std::vector<const CompiledFeature*> order;
    std::function<FeatureResult<void>(const CompiledFeature&)> visit = [&](const CompiledFeature& feature) -> FeatureResult<void> {
        const auto id = feature.descriptor.id;
        const auto state = visitState[id];
        if (state == 1) return std::unexpected(FeatureError{id, "Feature dependency cycle"});
        if (state == 2) return {};
        visitState[id] = 1;
        auto dependencies = feature.descriptor.dependencies;
        std::sort(dependencies.begin(), dependencies.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
        for (const auto& dependency : dependencies)
        {
            if (!ProjectAssets::IsStableIdentifier(dependency.id) || dependency.version == 0)
                return std::unexpected(FeatureError{id, "Feature dependency requirement is invalid"});
            auto found = entries.find(dependency.id);
            if (found == entries.end()) return std::unexpected(FeatureError{id, "required Feature is not compiled: " + dependency.id});
            if (found->second->descriptor.version != dependency.version)
                return std::unexpected(FeatureError{id, "required Feature version does not match: " + dependency.id});
            auto explicitVersion = requested.find(dependency.id);
            if (explicitVersion != requested.end() && explicitVersion->second != dependency.version)
                return std::unexpected(FeatureError{id, "manifest Feature version conflicts with dependency: " + dependency.id});
            requested.try_emplace(dependency.id, dependency.version);
            auto nested = visit(*found->second);
            if (!nested) return nested;
        }
        visitState[id] = 2;
        order.push_back(&feature);
        return {};
    };

    std::vector<std::string> roots;
    roots.reserve(requested.size());
    for (const auto& [id, version] : requested) { (void)version; roots.push_back(id); }
    std::sort(roots.begin(), roots.end());
    for (const auto& id : roots)
    {
        const auto found = entries.find(id);
        if (found == entries.end()) return std::unexpected(FeatureError{id, "required Feature is not compiled"});
        if (found->second->descriptor.version != requested.at(id)) return std::unexpected(FeatureError{id, "manifest Feature version does not match compiled version"});
        auto result = visit(*found->second);
        if (!result) return std::unexpected(result.error());
    }

    auto candidate = std::shared_ptr<RuntimeRegistry>(new RuntimeRegistry());
    candidate->m_Codecs = std::make_shared<Serialization::ComponentCodecRegistry>();
    candidate->m_AssetTypes = std::make_shared<ProjectAssets::AssetTypeRegistry>();
    for (const auto* feature : order)
    {
        auto [iter, inserted] = candidate->m_Features.emplace(feature->descriptor.id, feature->descriptor);
        if (!inserted) return std::unexpected(FeatureError{feature->descriptor.id, "duplicate Feature in activation order"});
        candidate->m_FeatureOrder.push_back(feature->descriptor.id);
        RuntimeFeatureRegistrar registrar(feature->descriptor, *candidate->m_Codecs, *candidate->m_AssetTypes,
                                          candidate->m_Components, candidate->m_Systems);
        auto result = feature->registerRuntime(registrar);
        if (!result) return std::unexpected(result.error());
    }
    candidate->m_Codecs->Freeze();
    candidate->m_AssetTypes->Freeze();
    return std::shared_ptr<const RuntimeRegistry>(std::move(candidate));
}

const FeatureDescriptor* RuntimeRegistry::FindFeature(std::string_view id) const noexcept
{
    const auto it = m_Features.find(std::string(id));
    return it == m_Features.end() ? nullptr : &it->second;
}

const RuntimeComponentDescriptor* RuntimeRegistry::FindComponent(std::string_view id) const noexcept
{
    const auto it = m_Components.find(std::string(id));
    return it == m_Components.end() ? nullptr : &it->second;
}

FeatureResult<void> RegisterBaseRuntimeComponents(RuntimeFeatureRegistrar& registrar)
{
    auto persistent = registrar.RegisterComponent<PersistentEntityIdComponent>();
    if (!persistent) return persistent;
    auto name = registrar.RegisterComponent<EntityNameComponent>();
    if (!name) return name;
    auto parent = registrar.RegisterComponent<ParentComponent>({}, [](const ParentComponent& component, const auto& visit) {
        if (component.parent.IsValid()) visit(component.parent);
    });
    if (!parent) return parent;
    return registrar.RegisterComponent<Serialization::ExcludeFromArchiveComponent, true>();
}
}
