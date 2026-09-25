#pragma once

#include <EditorFramework/Commands.h>
#include <GameFeature/RuntimeFeatureRegistrar.h>

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <variant>

namespace Aether::EditorFramework
{
enum class PropertyKind { Boolean, Integer, Float, String, Enumeration, Vector, AssetReference, FeaturePayload };

struct EnumerationValue { std::string value; };
struct VectorValue
{
    std::uint8_t dimensions = 0;
    std::array<double, 4> values{};
};
struct AssetReferenceValue
{
    std::optional<ProjectAssets::ProjectAssetRef> value;
};
struct FeaturePropertyPayload
{
    std::string typeId;
    Json payload;
};
using PropertyValue = std::variant<bool, std::int64_t, double, std::string, EnumerationValue,
                                   VectorValue, AssetReferenceValue, FeaturePropertyPayload>;

struct FieldDescriptor
{
    using Read = std::function<std::expected<PropertyValue, std::string>(const World&, EntityId)>;
    using Write = std::function<std::expected<PropertyValue, std::string>(World&, EntityId, const PropertyValue&)>;
    std::string id;
    std::string label;
    PropertyKind kind = PropertyKind::String;
    std::string expectedAssetType;
    bool readOnly = false;
    bool nullable = true;
    std::optional<double> minimum;
    std::optional<double> maximum;
    std::vector<std::string> enumValues;
    Read read;
    Write write;
    std::function<Json(const PropertyValue&)> encode;
    std::function<std::expected<PropertyValue, std::string>(const Json&)> decode;
    std::function<std::expected<void, std::string>(const PropertyValue&)> validateCustom;

    std::expected<PropertyValue, std::string> ReadValue(const World& world, EntityId entity) const;
    std::expected<PropertyValue, std::string> SetValue(World& world, EntityId entity, const PropertyValue& value) const;
    std::expected<PropertyValue, std::string> DecodeValue(const Json& json) const;
    Json EncodeValue(const PropertyValue& value) const;
};

struct ComponentEditorDescriptor
{
    GameFeatures::FeatureId ownerFeature;
    GameFeatures::ComponentTypeId componentType;
    std::function<bool(const World&, EntityId)> hasComponent;
    std::function<std::expected<void, std::string>(World&, EntityId)> addComponent;
    std::function<std::expected<void, std::string>(World&, EntityId)> removeComponent;
    std::vector<FieldDescriptor> fields;
    const FieldDescriptor* FindField(std::string_view fieldId) const noexcept;
};

class ComponentSchemaRegistry
{
public:
    std::expected<void, std::string> Register(ComponentEditorDescriptor descriptor,
                                               const GameFeatures::RuntimeRegistry& runtime);
    void Freeze() noexcept { m_Frozen = true; }
    bool Frozen() const noexcept { return m_Frozen; }
    const ComponentEditorDescriptor* Find(std::string_view componentType) const noexcept;
    const auto& Entries() const noexcept { return m_Entries; }
private:
    bool m_Frozen = false;
    std::unordered_map<std::string, ComponentEditorDescriptor> m_Entries;
};

struct InspectorTarget
{
    GameFeatures::WorldInstanceId worldInstanceId;
    GameFeatures::PersistentEntityId entityId;
};
struct InspectorFieldModel
{
    std::string fieldId;
    std::string label;
    PropertyKind kind = PropertyKind::String;
    std::string expectedAssetType;
    bool readOnly = true;
    std::string actualAssetType;
    std::string actualAssetId;
    std::string actualDisplayPath;
    std::optional<PropertyValue> value;
    std::string error;
};
struct InspectorComponentModel
{
    GameFeatures::ComponentTypeId componentType;
    bool readOnly = true;
    std::vector<InspectorFieldModel> fields;
};
struct InspectorModel
{
    bool validTarget = false;
    std::string error;
    std::vector<InspectorComponentModel> components;
};

InspectorModel BuildInspectorModel(const World& world, const InspectorTarget& target,
    GameFeatures::WorldInstanceId currentWorldInstance, const GameFeatures::RuntimeRegistry& runtime,
    const ComponentSchemaRegistry& schemas, const ProjectAssets::CatalogSnapshot& catalog);
std::expected<void, std::string> RegisterInspectorCommands(CommandRegistry& commands,
    std::shared_ptr<const ComponentSchemaRegistry> schemas,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime = {});
}
