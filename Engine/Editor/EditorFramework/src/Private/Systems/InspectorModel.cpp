#include <EditorFramework/ComponentSchema.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Aether::EditorFramework
{
namespace
{
bool Matches(PropertyKind kind, const PropertyValue& value)
{
    switch (kind)
    {
    case PropertyKind::Boolean: return std::holds_alternative<bool>(value);
    case PropertyKind::Integer: return std::holds_alternative<std::int64_t>(value);
    case PropertyKind::Float: return std::holds_alternative<double>(value);
    case PropertyKind::String: return std::holds_alternative<std::string>(value);
    case PropertyKind::Enumeration: return std::holds_alternative<EnumerationValue>(value);
    case PropertyKind::Vector: return std::holds_alternative<VectorValue>(value);
    case PropertyKind::AssetReference: return std::holds_alternative<AssetReferenceValue>(value);
    case PropertyKind::FeaturePayload: return std::holds_alternative<FeaturePropertyPayload>(value);
    }
    return false;
}

std::expected<void, std::string> Validate(const FieldDescriptor& field, const PropertyValue& value)
{
    if (!Matches(field.kind, value)) return std::unexpected("property value does not match field kind");
    if (field.kind == PropertyKind::Float)
    {
        const auto numeric = std::get<double>(value);
        if (!std::isfinite(numeric)) return std::unexpected("float property must be finite");
        if (field.minimum && numeric < *field.minimum) return std::unexpected("float property is below field minimum");
        if (field.maximum && numeric > *field.maximum) return std::unexpected("float property is above field maximum");
    }
    else if (field.kind == PropertyKind::Integer)
    {
        const auto numeric = static_cast<double>(std::get<std::int64_t>(value));
        if (field.minimum && numeric < *field.minimum) return std::unexpected("integer property is below field minimum");
        if (field.maximum && numeric > *field.maximum) return std::unexpected("integer property is above field maximum");
    }
    else if (field.kind == PropertyKind::Enumeration)
    {
        const auto& current = std::get<EnumerationValue>(value).value;
        if (std::find(field.enumValues.begin(), field.enumValues.end(), current) == field.enumValues.end())
            return std::unexpected("enum property contains an unregistered value");
    }
    else if (field.kind == PropertyKind::Vector)
    {
        const auto& vector = std::get<VectorValue>(value);
        if (vector.dimensions < 2 || vector.dimensions > 4) return std::unexpected("vector property must have 2, 3, or 4 dimensions");
        for (std::uint8_t i = 0; i < vector.dimensions; ++i)
        {
            const auto item = vector.values[i];
            if (!std::isfinite(item)) return std::unexpected("vector values must be finite");
            if (field.minimum && item < *field.minimum) return std::unexpected("vector value is below field minimum");
            if (field.maximum && item > *field.maximum) return std::unexpected("vector value is above field maximum");
        }
    }
    else if (field.kind == PropertyKind::AssetReference)
    {
        const auto& ref = std::get<AssetReferenceValue>(value).value;
        if (!ref && !field.nullable) return std::unexpected("asset reference field cannot be empty");
        if (ref && (!ref->project.IsValid() || !ref->asset.IsValid())) return std::unexpected("asset reference contains an invalid ID");
    }
    else if (field.kind == PropertyKind::FeaturePayload)
    {
        const auto& feature = std::get<FeaturePropertyPayload>(value);
        if (!ProjectAssets::IsStableIdentifier(feature.typeId)) return std::unexpected("feature payload type ID is invalid");
    }
    if (field.validateCustom)
    {
        auto custom = field.validateCustom(value);
        if (!custom) return custom;
    }
    return {};
}

std::expected<std::string, std::string> ReadText(const Json& payload, std::string_view key)
{
    if (!payload.is_object() || !payload.contains(key) || !payload[key].is_string())
        return std::unexpected("command payload requires string field '" + std::string(key) + "'");
    return payload[key].get<std::string>();
}
}

std::expected<PropertyValue, std::string> FieldDescriptor::ReadValue(const World& world, EntityId entity) const
{
    if (!read) return std::unexpected("field has no read thunk");
    auto value = read(world, entity);
    if (!value) return std::unexpected(value.error());
    auto valid = Validate(*this, *value);
    if (!valid) return std::unexpected(valid.error());
    return value;
}

std::expected<PropertyValue, std::string> FieldDescriptor::SetValue(World& world, EntityId entity, const PropertyValue& value) const
{
    if (!write) return std::unexpected("field is read-only");
    auto valid = Validate(*this, value);
    if (!valid) return std::unexpected(valid.error());
    auto previous = write(world, entity, value);
    if (!previous) return std::unexpected(previous.error());
    auto previousValid = Validate(*this, *previous);
    if (!previousValid) return std::unexpected("field read thunk returned an invalid previous value: " + previousValid.error());
    return previous;
}

std::expected<PropertyValue, std::string> FieldDescriptor::DecodeValue(const Json& json) const
{
    if (decode)
    {
        auto value = decode(json);
        if (!value) return value;
        auto valid = Validate(*this, *value);
        if (!valid) return std::unexpected(valid.error());
        return value;
    }
    PropertyValue value;
    switch (kind)
    {
    case PropertyKind::Boolean:
        if (!json.is_boolean()) return std::unexpected("expected a boolean property value");
        value = json.get<bool>(); break;
    case PropertyKind::Integer:
        if (!json.is_number_integer()) return std::unexpected("expected an integer property value");
        value = json.get<std::int64_t>(); break;
    case PropertyKind::Float:
        if (!json.is_number()) return std::unexpected("expected a numeric property value");
        value = json.get<double>(); break;
    case PropertyKind::String:
        if (!json.is_string()) return std::unexpected("expected a string property value");
        value = json.get<std::string>(); break;
    case PropertyKind::Enumeration:
        if (!json.is_string()) return std::unexpected("expected an enum string value");
        value = EnumerationValue{json.get<std::string>()}; break;
    case PropertyKind::Vector:
    {
        if (!json.is_array() || json.size() < 2 || json.size() > 4) return std::unexpected("vector property must have 2, 3, or 4 numbers");
        VectorValue vector;
        vector.dimensions = static_cast<std::uint8_t>(json.size());
        for (std::size_t i = 0; i < json.size(); ++i)
        {
            if (!json[i].is_number()) return std::unexpected("vector property entries must be numbers");
            vector.values[i] = json[i].get<double>();
        }
        value = vector;
        break;
    }
    case PropertyKind::AssetReference:
        if (json.is_null()) value = AssetReferenceValue{};
        else
        {
            if (!json.is_object() || !json.contains("projectId") || !json["projectId"].is_string()
                || !json.contains("assetId") || !json["assetId"].is_string())
                return std::unexpected("asset reference requires projectId and assetId");
            auto project = ProjectAssets::ProjectId::Parse(json["projectId"].get<std::string>());
            auto asset = ProjectAssets::AssetId::Parse(json["assetId"].get<std::string>());
            if (!project || !asset) return std::unexpected("asset reference IDs are not canonical UUIDs");
            value = AssetReferenceValue{ProjectAssets::ProjectAssetRef{*project, *asset}};
        }
        break;
    case PropertyKind::FeaturePayload:
        if (!json.is_object() || !json.contains("typeId") || !json["typeId"].is_string() || !json.contains("payload"))
            return std::unexpected("feature payload requires typeId and payload");
        value = FeaturePropertyPayload{json["typeId"].get<std::string>(), json["payload"]};
        break;
    }
    auto valid = Validate(*this, value);
    if (!valid) return std::unexpected(valid.error());
    return value;
}

Json FieldDescriptor::EncodeValue(const PropertyValue& value) const
{
    if (encode) return encode(value);
    switch (kind)
    {
    case PropertyKind::Boolean: return std::get<bool>(value);
    case PropertyKind::Integer: return std::get<std::int64_t>(value);
    case PropertyKind::Float: return std::get<double>(value);
    case PropertyKind::String: return std::get<std::string>(value);
    case PropertyKind::Enumeration: return std::get<EnumerationValue>(value).value;
    case PropertyKind::Vector:
    {
        const auto& vector = std::get<VectorValue>(value);
        Json result = Json::array();
        for (std::uint8_t i = 0; i < vector.dimensions; ++i) result.push_back(vector.values[i]);
        return result;
    }
    case PropertyKind::AssetReference:
    {
        const auto& ref = std::get<AssetReferenceValue>(value).value;
        if (!ref) return nullptr;
        return {{"projectId", ref->project.ToString()}, {"assetId", ref->asset.ToString()}};
    }
    case PropertyKind::FeaturePayload:
    {
        const auto& payload = std::get<FeaturePropertyPayload>(value);
        return {{"typeId", payload.typeId}, {"payload", payload.payload}};
    }
    }
    return nullptr;
}

const FieldDescriptor* ComponentEditorDescriptor::FindField(std::string_view fieldId) const noexcept
{
    const auto it = std::find_if(fields.begin(), fields.end(), [&](const auto& field) { return field.id == fieldId; });
    return it == fields.end() ? nullptr : &*it;
}

std::expected<void, std::string> ComponentSchemaRegistry::Register(ComponentEditorDescriptor descriptor,
                                                                     const GameFeatures::RuntimeRegistry& runtime)
{
    if (m_Frozen) return std::unexpected("component schema registry is frozen");
    if (!ProjectAssets::IsStableIdentifier(descriptor.componentType) || !ProjectAssets::IsStableIdentifier(descriptor.ownerFeature)
        || !descriptor.hasComponent)
        return std::unexpected("component schema requires a stable ID, owner, and presence thunk");
    if (static_cast<bool>(descriptor.addComponent) != static_cast<bool>(descriptor.removeComponent))
        return std::unexpected("component schema must provide both add and remove actions, or neither");
    const auto* runtimeComponent = runtime.FindComponent(descriptor.componentType);
    if (!runtimeComponent) return std::unexpected("component schema has no matching Runtime component");
    if (runtimeComponent->ownerFeature != descriptor.ownerFeature)
        return std::unexpected("component schema owner differs from Runtime component owner");
    if (m_Entries.contains(descriptor.componentType)) return std::unexpected("component schema is already registered");
    std::unordered_map<std::string, const GameFeatures::AssetFieldContract*> assetFields;
    for (const auto& item : runtimeComponent->assetFields) assetFields.emplace(item.fieldId, &item);
    std::unordered_set<std::string> ids;
    for (const auto& field : descriptor.fields)
    {
        if (!ProjectAssets::IsStableIdentifier(field.id) || field.label.empty() || !field.read
            || (!field.readOnly && !field.write))
            return std::unexpected("schema field requires a stable ID, label, read thunk, and writable fields require a write thunk");
        if (!ids.emplace(field.id).second) return std::unexpected("schema contains a duplicate field ID");
        if (field.minimum && !std::isfinite(*field.minimum)) return std::unexpected("field minimum must be finite");
        if (field.maximum && !std::isfinite(*field.maximum)) return std::unexpected("field maximum must be finite");
        if (field.minimum && field.maximum && *field.minimum > *field.maximum) return std::unexpected("field minimum exceeds maximum");
        if (field.kind == PropertyKind::AssetReference)
        {
            if (!ProjectAssets::IsStableIdentifier(field.expectedAssetType)) return std::unexpected("asset field expected type is invalid");
            const auto contract = assetFields.find(field.id);
            if (contract == assetFields.end() || contract->second->expectedType != field.expectedAssetType
                || contract->second->nullable != field.nullable)
                return std::unexpected("asset field schema differs from Runtime reference contract");
        }
        else if (!field.expectedAssetType.empty()) return std::unexpected("only asset-reference fields may declare an expected type");
        for (const auto& enumValue : field.enumValues)
            if (enumValue.empty()) return std::unexpected("enum values cannot be empty");
        if (field.kind == PropertyKind::Enumeration && field.enumValues.empty())
            return std::unexpected("enum fields require at least one allowed value");
    }
    if (ids.size() != descriptor.fields.size()) return std::unexpected("component schema has invalid field IDs");
    for (const auto& [id, contract] : assetFields)
    {
        (void)contract;
        const auto* field = descriptor.FindField(id);
        if (!field || field->kind != PropertyKind::AssetReference)
            return std::unexpected("schema omitted a Runtime asset field: " + id);
    }
    m_Entries.emplace(descriptor.componentType, std::move(descriptor));
    return {};
}

const ComponentEditorDescriptor* ComponentSchemaRegistry::Find(std::string_view componentType) const noexcept
{
    const auto it = m_Entries.find(std::string(componentType));
    return it == m_Entries.end() ? nullptr : &it->second;
}

InspectorModel BuildInspectorModel(const World& world, const InspectorTarget& target,
    GameFeatures::WorldInstanceId currentWorldInstance, const GameFeatures::RuntimeRegistry& runtime,
    const ComponentSchemaRegistry& schemas, const ProjectAssets::CatalogSnapshot& catalog)
{
    InspectorModel model;
    if (!target.worldInstanceId.IsValid() || target.worldInstanceId != currentWorldInstance)
    {
        model.error = "selection belongs to a stale World instance";
        return model;
    }
    EntityId entity = entt::null;
    for (const auto candidate : world.Select<GameFeatures::PersistentEntityIdComponent>())
        if (world.GetComponent<GameFeatures::PersistentEntityIdComponent>(candidate).value == target.entityId)
        {
            entity = candidate;
            break;
        }
    if (entity == entt::null)
    {
        model.error = "selected entity no longer exists";
        return model;
    }
    model.validTarget = true;
    std::vector<std::string> types;
    for (const auto& [type, descriptor] : runtime.Components())
        if (descriptor.hasComponent && descriptor.hasComponent(world, entity)) types.push_back(type);
    std::sort(types.begin(), types.end());
    for (const auto& type : types)
    {
        InspectorComponentModel component;
        component.componentType = type;
        const auto* schema = schemas.Find(type);
        if (!schema)
        {
            model.components.push_back(std::move(component));
            continue;
        }
        component.readOnly = std::all_of(schema->fields.begin(), schema->fields.end(), [](const auto& field) { return field.readOnly; });
        for (const auto& field : schema->fields)
        {
            InspectorFieldModel fieldModel;
            fieldModel.fieldId = field.id;
            fieldModel.label = field.label;
            fieldModel.kind = field.kind;
            fieldModel.expectedAssetType = field.expectedAssetType;
            fieldModel.readOnly = field.readOnly;
            try
            {
                auto value = field.ReadValue(world, entity);
                if (!value) fieldModel.error = value.error();
                else
                {
                    fieldModel.value = std::move(*value);
                    if (field.kind == PropertyKind::AssetReference)
                    {
                        const auto& ref = std::get<AssetReferenceValue>(*fieldModel.value).value;
                        if (ref)
                        {
                            if (ref->project != catalog.Project()) fieldModel.error = "asset belongs to another project";
                            else if (const auto* record = catalog.Find(ref->asset))
                            {
                                fieldModel.actualAssetId = record->id.ToString();
                                fieldModel.actualAssetType = record->type;
                                fieldModel.actualDisplayPath = record->displayPath;
                                if (record->type != field.expectedAssetType) fieldModel.error = "asset type does not match field";
                            }
                            else fieldModel.error = "asset is missing from the project catalog";
                        }
                    }
                }
            }
            catch (const std::exception& exception) { fieldModel.error = exception.what(); }
            component.fields.push_back(std::move(fieldModel));
        }
        model.components.push_back(std::move(component));
    }
    return model;
}

std::expected<void, std::string> RegisterInspectorCommands(CommandRegistry& commands,
    std::shared_ptr<const ComponentSchemaRegistry> schemas,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime)
{
    if (!schemas) return std::unexpected("Inspector commands require a component schema registry");
    CommandDescriptor descriptor;
    descriptor.id = "aether.component.set-field";
    descriptor.apply = [schemas](World& world, const Json& payload) -> std::expected<EditOperation, std::string> {
        auto entityId = ReadText(payload, "entityId");
        auto type = ReadText(payload, "componentType");
        auto fieldId = ReadText(payload, "fieldId");
        if (!entityId) return std::unexpected(entityId.error());
        if (!type) return std::unexpected(type.error());
        if (!fieldId) return std::unexpected(fieldId.error());
        auto persistentId = GameFeatures::PersistentEntityId::Parse(*entityId);
        if (!persistentId) return std::unexpected("entityId is not a canonical UUID");
        EntityId entity = entt::null;
        for (const auto candidate : world.Select<GameFeatures::PersistentEntityIdComponent>())
            if (world.GetComponent<GameFeatures::PersistentEntityIdComponent>(candidate).value == *persistentId)
            {
                entity = candidate;
                break;
            }
        if (entity == entt::null) return std::unexpected("entity does not exist");
        const auto* schema = schemas->Find(*type);
        if (!schema || !schema->hasComponent(world, entity)) return std::unexpected("component has no editable schema on selected entity");
        const auto* field = schema->FindField(*fieldId);
        if (!field || !payload.contains("value")) return std::unexpected("field or value is missing from command payload");
        auto value = field->DecodeValue(payload["value"]);
        if (!value) return std::unexpected(value.error());
        auto previous = field->SetValue(world, entity, *value);
        if (!previous) return std::unexpected(previous.error());
        return EditOperation{"aether.component.set-field", {{"entityId", *entityId}, {"componentType", *type},
            {"fieldId", *fieldId}, {"value", field->EncodeValue(*previous)}}};
    };
    auto fieldCommand = commands.Register(std::move(descriptor));
    if (!fieldCommand) return fieldCommand;
    if (!runtime) return {};

    auto resolveEntity = [](World& world, std::string_view entityId) -> std::optional<EntityId> {
        auto persistentId = GameFeatures::PersistentEntityId::Parse(entityId);
        if (!persistentId) return std::nullopt;
        for (const auto entity : world.Select<GameFeatures::PersistentEntityIdComponent>())
            if (world.GetComponent<GameFeatures::PersistentEntityIdComponent>(entity).value == *persistentId)
                return entity;
        return std::nullopt;
    };
    auto captureWorld = [runtime](World& world) -> std::expected<Json, std::string> {
        Serialization::SaveContext context;
        auto archive = world.Serialize(runtime->Codecs(), context);
        if (!archive) return std::unexpected(archive.error().message);
        return std::move(*archive);
    };
    for (const auto [id, add] : {std::pair{"aether.component.add", true},
                                 std::pair{"aether.component.remove", false}})
    {
        CommandDescriptor componentCommand;
        componentCommand.id = id;
        componentCommand.apply = [schemas, resolveEntity, captureWorld, add](World& world, const Json& payload)
            -> std::expected<EditOperation, std::string> {
            auto entityText = ReadText(payload, "entityId");
            auto componentType = ReadText(payload, "componentType");
            if (!entityText) return std::unexpected(entityText.error());
            if (!componentType) return std::unexpected(componentType.error());
            auto entity = resolveEntity(world, *entityText);
            if (!entity) return std::unexpected("entity does not exist or has an invalid PersistentEntityId");
            const auto* schema = schemas->Find(*componentType);
            if (!schema) return std::unexpected("component has no registered Editor schema");
            if (add && !schema->addComponent) return std::unexpected("component cannot be added from the Inspector");
            if (!add && !schema->removeComponent) return std::unexpected("component cannot be removed from the Inspector");
            if (add && schema->hasComponent(world, *entity)) return std::unexpected("component is already present");
            if (!add && !schema->hasComponent(world, *entity)) return std::unexpected("component is not present");
            auto before = captureWorld(world);
            if (!before) return std::unexpected(before.error());
            auto changed = add ? schema->addComponent(world, *entity) : schema->removeComponent(world, *entity);
            if (!changed) return std::unexpected(changed.error());
            if (schema->hasComponent(world, *entity) != add)
                return std::unexpected("component action did not produce its declared result");
            return EditOperation{"aether.restore-world-state", {{"archive", std::move(*before)}}};
        };
        auto registered = commands.Register(std::move(componentCommand));
        if (!registered) return registered;
    }
    return {};
}
}
