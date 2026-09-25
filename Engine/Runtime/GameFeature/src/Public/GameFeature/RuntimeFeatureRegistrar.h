#pragma once

#include <GameFeature/WorldMount.h>
#include <ProjectAsset/AssetTypeRegistry.h>
#include <ProjectAsset/ProjectManifest.h>
#include <World/Serialization/ComponentCodecRegistry.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace Aether::GameFeatures
{
struct AssetFieldContract
{
    std::string fieldId;
    ProjectAssets::AssetTypeId expectedType;
    bool nullable = true;
};

struct AssetReferenceValue
{
    std::string fieldId;
    ProjectAssets::AssetTypeId expectedType;
    std::optional<ProjectAssets::ProjectAssetRef> value;
};

struct RuntimeComponentDescriptor
{
    ComponentTypeId id;
    FeatureId ownerFeature;
    std::vector<AssetFieldContract> assetFields;
    std::function<void(const World&, EntityId, const std::function<void(const PersistentEntityId&)>&)> visitEntityReferences;
    std::function<void(const World&, EntityId, const std::function<void(const AssetReferenceValue&)>&)> visitAssetReferences;
    std::function<bool(const World&, EntityId)> hasComponent;
    std::function<Serialization::Result<void>(const World&, EntityId)> validate;
};

struct FeatureError
{
    std::string featureId;
    std::string message;
};
template<class T>
using FeatureResult = std::expected<T, FeatureError>;

class RuntimeFeatureRegistrar
{
public:
    using ReferenceCallback = std::function<void(const PersistentEntityId&)>;
    using AssetReferenceCallback = std::function<void(const AssetReferenceValue&)>;
    template<class T, bool Transient = false>
    FeatureResult<void> RegisterComponent(std::vector<AssetFieldContract> fields = {},
        std::function<void(const T&, const ReferenceCallback&)> entityReferences = {},
        std::function<void(const T&, const AssetReferenceCallback&)> assetReferences = {},
        std::function<Serialization::Result<void>(const T&)> validator = {})
    {
        if (m_Frozen) return std::unexpected(FeatureError{m_Feature.id, "runtime registrar is frozen"});
        using Codec = Serialization::ComponentSerialization<T>;
        const std::string id(Codec::Type);
        if (!ProjectAssets::IsStableIdentifier(id)) return std::unexpected(FeatureError{m_Feature.id, "component type ID is invalid: " + id});
        if (m_Components.contains(id)) return std::unexpected(FeatureError{m_Feature.id, "component type is already registered: " + id});
        for (const auto& field : fields)
            if (!ProjectAssets::IsStableIdentifier(field.fieldId) || !ProjectAssets::IsStableIdentifier(field.expectedType))
                return std::unexpected(FeatureError{m_Feature.id, "asset field contract is invalid on " + id});
        std::unordered_set<std::string> fieldIds;
        for (const auto& field : fields)
            if (!fieldIds.emplace(field.fieldId).second)
                return std::unexpected(FeatureError{m_Feature.id, "asset field ID is duplicated on " + id});
        if (!fields.empty() && !assetReferences)
            return std::unexpected(FeatureError{m_Feature.id, "asset fields require a reference visitor on " + id});
        Serialization::Result<void> codec;
        if constexpr (Transient) codec = m_Codecs.RegisterTransient<T>();
        else codec = m_Codecs.Register<T>();
        if (!codec) return std::unexpected(FeatureError{m_Feature.id, codec.error().message});

        RuntimeComponentDescriptor descriptor;
        descriptor.id = id;
        descriptor.ownerFeature = m_Feature.id;
        descriptor.assetFields = std::move(fields);
        descriptor.hasComponent = [](const World& world, EntityId entity) { return world.HasComponent<T>(entity); };
        if (assetReferences)
        {
            descriptor.visitAssetReferences = [visitor = std::move(assetReferences)](const World& world, EntityId entity, const AssetReferenceCallback& callback) {
                visitor(world.GetComponent<T>(entity), callback);
            };
        }
        if (entityReferences)
        {
            descriptor.visitEntityReferences = [visitor = std::move(entityReferences)](const World& world, EntityId entity, const ReferenceCallback& callback) {
                visitor(world.GetComponent<T>(entity), callback);
            };
        }
        if (validator)
        {
            descriptor.validate = [check = std::move(validator)](const World& world, EntityId entity) { return check(world.GetComponent<T>(entity)); };
        }
        else
        {
            descriptor.validate = [](const World&, EntityId) -> Serialization::Result<void> { return {}; };
        }
        m_Components.emplace(std::move(id), std::move(descriptor));
        return {};
    }

    ProjectAssets::Result<void> RegisterAssetType(ProjectAssets::AssetTypeDescriptor descriptor);
    FeatureResult<void> RegisterSystem(SystemRegistration registration);
    const FeatureDescriptor& Feature() const noexcept { return m_Feature; }
private:
    friend class RuntimeRegistry;
    RuntimeFeatureRegistrar(FeatureDescriptor feature, Serialization::ComponentCodecRegistry& codecs,
                           ProjectAssets::AssetTypeRegistry& assets,
                           std::unordered_map<std::string, RuntimeComponentDescriptor>& components,
                           std::vector<SystemRegistration>& systems);
    FeatureDescriptor m_Feature;
    Serialization::ComponentCodecRegistry& m_Codecs;
    ProjectAssets::AssetTypeRegistry& m_Assets;
    std::unordered_map<std::string, RuntimeComponentDescriptor>& m_Components;
    std::vector<SystemRegistration>& m_Systems;
    bool m_Frozen = false;
};

struct CompiledFeature
{
    FeatureDescriptor descriptor;
    std::function<FeatureResult<void>(RuntimeFeatureRegistrar&)> registerRuntime;
};

class RuntimeRegistry
{
public:
    static FeatureResult<std::shared_ptr<const RuntimeRegistry>> Build(
        const ProjectAssets::ProjectManifest& manifest, std::vector<CompiledFeature> compiled);
    const FeatureDescriptor* FindFeature(std::string_view id) const noexcept;
    const RuntimeComponentDescriptor* FindComponent(std::string_view id) const noexcept;
    const auto& Components() const noexcept { return m_Components; }
    const std::vector<SystemRegistration>& Systems() const noexcept { return m_Systems; }
    const Serialization::ComponentCodecRegistry& Codecs() const noexcept { return *m_Codecs; }
    const ProjectAssets::AssetTypeRegistry& AssetTypes() const noexcept { return *m_AssetTypes; }
    const std::vector<FeatureId>& FeatureOrder() const noexcept { return m_FeatureOrder; }
private:
    std::shared_ptr<Serialization::ComponentCodecRegistry> m_Codecs;
    std::shared_ptr<ProjectAssets::AssetTypeRegistry> m_AssetTypes;
    std::unordered_map<std::string, FeatureDescriptor> m_Features;
    std::unordered_map<std::string, RuntimeComponentDescriptor> m_Components;
    std::vector<SystemRegistration> m_Systems;
    std::vector<FeatureId> m_FeatureOrder;
};

FeatureResult<void> RegisterBaseRuntimeComponents(RuntimeFeatureRegistrar& registrar);
}
