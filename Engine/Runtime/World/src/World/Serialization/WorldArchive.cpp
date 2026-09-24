#include <World/Serialization/WorldArchive.h>

#include <algorithm>
#include <limits>
#include <new>
#include <set>

namespace Aether::Serialization
{
namespace
{
ArchiveError Error(ArchiveErrorCode code, std::string message, std::string pointer = {})
{
    ArchiveError error;
    error.code = code;
    error.message = std::move(message);
    error.jsonPointer = std::move(pointer);
    return error;
}

Result<std::uint32_t> ReadVersion(const Json& value, std::string pointer)
{
    if (!(value.is_number_integer() || value.is_number_unsigned()))
        return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "version must be an integer", std::move(pointer)));
    if (value.is_number_integer())
    {
        const auto number = value.get<std::int64_t>();
        if (number <= 0 || number > std::numeric_limits<std::uint32_t>::max())
            return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "version is out of range", std::move(pointer)));
        return static_cast<std::uint32_t>(number);
    }
    const auto number = value.get<std::uint64_t>();
    if (number == 0 || number > std::numeric_limits<std::uint32_t>::max())
        return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "version is out of range", std::move(pointer)));
    return static_cast<std::uint32_t>(number);
}
} // namespace

const ComponentCodecRegistry::Entry* ComponentCodecRegistry::Find(std::string_view type) const noexcept
{
    const auto it = m_Entries.find(std::string(type));
    return it == m_Entries.end() ? nullptr : &it->second;
}

const ComponentCodecRegistry::Entry* ComponentCodecRegistry::Find(const entt::type_info& runtimeType) const noexcept
{
    for (const auto& [type, entry] : m_Entries)
    {
        (void)type;
        if (*entry.runtimeType == runtimeType) return &entry;
    }
    return nullptr;
}

bool ComponentCodecRegistry::HasRuntimeType(const entt::type_info& runtimeType) const noexcept
{
    return Find(runtimeType) != nullptr;
}
} // namespace Aether::Serialization

namespace Aether
{
class WorldArchiveAccess
{
public:
    static bool IsDispatching(const World& world) { return world.m_DispatchDepth != 0; }
    static std::vector<std::pair<const entt::type_info*, std::size_t>> Storages(const World& world)
    {
        std::vector<std::pair<const entt::type_info*, std::size_t>> result;
        for (auto&& [id, storage] : world.m_Registry.storage())
        {
            (void)id;
            result.emplace_back(&storage.type(), storage.size());
        }
        return result;
    }
    static bool HasComponentByType(const World& world, EntityId entity, const entt::type_info& type)
    {
        for (auto&& [id, storage] : world.m_Registry.storage())
        {
            (void)id;
            if (storage.type() == type)
            {
                const auto* sparse = static_cast<const entt::basic_sparse_set<entt::entity>*>(&storage);
                return sparse->contains(entity);
            }
        }
        return false;
    }
};

Serialization::Result<Json> World::Serialize(const Serialization::ComponentCodecRegistry& codecs,
                                             Serialization::SaveContext& context) const
{
    using namespace Serialization;
    std::string stage = "initializing serialization";
    try
    {
        if (WorldArchiveAccess::IsDispatching(*this))
            return std::unexpected(Error(ArchiveErrorCode::InvalidArgument, "cannot serialize a World during a system callback"));
        codecs.Freeze();
        stage = "enumerating entities";
        const auto entities = Entities();
        if (entities.size() > context.m_Limits.maxEntities)
            return std::unexpected(Error(ArchiveErrorCode::ResourceLimitExceeded, "entity count exceeds archive limit"));

        stage = "checking component storage coverage";
        for (const auto& [runtimeType, count] : WorldArchiveAccess::Storages(*this))
        {
            if (count == 0 || *runtimeType == entt::type_id<EntityId>()) continue;
            const auto* entry = codecs.Find(*runtimeType);
            if (!entry)
                return std::unexpected(Error(ArchiveErrorCode::UnknownComponent, "World contains a component type without a codec"));
        }

        auto sortedEntities = entities;
        std::sort(sortedEntities.begin(), sortedEntities.end(), [](EntityId left, EntityId right) {
            return entt::to_integral(left) < entt::to_integral(right);
        });
        stage = "building internal entity IDs";
        context.m_EntityIds.clear();
        for (std::size_t i = 0; i < sortedEntities.size(); ++i)
        {
            std::string id = "e";
            const auto ordinal = std::to_string(i + 1);
            id.append(6 > ordinal.size() ? 6 - ordinal.size() : 0, '0');
            id += ordinal;
            context.m_EntityIds.emplace(static_cast<std::uint32_t>(entt::to_integral(sortedEntities[i])), std::move(id));
        }

        Json result = Json::object();
        result["entities"] = Json::array();
        std::uint64_t totalComponents = 0;
        for (const auto entity : sortedEntities)
        {
            stage = "encoding entity record";
            Json entityRecord = Json::object();
            auto id = context.EncodeEntity(entity);
            if (!id) return std::unexpected(id.error());
            const auto entityId = id->get<std::string>();
            entityRecord["id"] = std::move(*id);
            entityRecord["components"] = Json::array();

            std::vector<const ComponentCodecRegistry::Entry*> entries;
            entries.reserve(codecs.Entries().size());
            for (const auto& [type, entry] : codecs.Entries())
            {
                (void)type;
                if (!entry.transient && WorldArchiveAccess::HasComponentByType(*this, entity, *entry.runtimeType)) entries.push_back(&entry);
            }
            if (entries.size() > context.m_Limits.maxComponentsPerEntity)
                return std::unexpected(Error(ArchiveErrorCode::ResourceLimitExceeded, "components per entity exceed archive limit"));
            if (entries.size() > context.m_Limits.maxComponents || totalComponents > context.m_Limits.maxComponents - entries.size())
                return std::unexpected(Error(ArchiveErrorCode::ResourceLimitExceeded, "component count exceeds archive limit"));
            totalComponents += entries.size();
            std::sort(entries.begin(), entries.end(), [](const auto* left, const auto* right) { return left->type < right->type; });
            for (const auto* entry : entries)
            {
                stage = "encoding component " + entry->type;
                Json component = Json::object();
                component["type"] = entry->type;
                component["version"] = entry->version;
                Serialization::Result<Json> data = std::unexpected(Error(ArchiveErrorCode::CodecFailure, "component codec did not return a result"));
                try
                {
                    data = entry->save(*this, entity, context);
                }
                catch (const std::bad_alloc&)
                {
                    auto error = Error(ArchiveErrorCode::AllocationFailure, "allocation failed in component codec");
                    error.entityId = entityId;
                    error.componentType = entry->type;
                    error.jsonPointer = "/entities/" + std::to_string(result["entities"].size()) + "/components/" + std::to_string(entityRecord["components"].size()) + "/data";
                    return std::unexpected(std::move(error));
                }
                catch (const std::exception& exception)
                {
                    auto error = Error(ArchiveErrorCode::CodecFailure, exception.what());
                    error.entityId = entityId;
                    error.componentType = entry->type;
                    error.jsonPointer = "/entities/" + std::to_string(result["entities"].size()) + "/components/" + std::to_string(entityRecord["components"].size()) + "/data";
                    return std::unexpected(std::move(error));
                }
                if (!data)
                {
                    auto error = data.error();
                    error.entityId = entityId;
                    error.componentType = entry->type;
                    if (error.jsonPointer.empty()) error.jsonPointer = "/entities/" + std::to_string(result["entities"].size()) + "/components/" + std::to_string(entityRecord["components"].size()) + "/data";
                    return std::unexpected(std::move(error));
                }
                component["data"] = std::move(*data);
                entityRecord["components"].push_back(std::move(component));
            }
            result["entities"].push_back(std::move(entityRecord));
        }
        return result;
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error(ArchiveErrorCode::AllocationFailure, "allocation failed while serializing World"));
    }
    catch (const std::exception& exception)
    {
        return std::unexpected(Error(ArchiveErrorCode::CodecFailure, stage + ": " + exception.what()));
    }
}

Serialization::Result<std::unique_ptr<World>> World::Deserialize(const Json& document,
                                                   const Serialization::ComponentCodecRegistry& codecs,
                                                   Serialization::LoadContext& context)
{
    using namespace Serialization;
    try
    {
        codecs.Freeze();
        if (!document.is_object() || !document.contains("entities") || !document["entities"].is_array())
            return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "World document requires an entities array", "/entities"));
        const auto& entities = document["entities"];
        if (entities.size() > context.m_Limits.maxEntities)
            return std::unexpected(Error(ArchiveErrorCode::ResourceLimitExceeded, "entity count exceeds archive limit", "/entities"));

        struct PendingComponent { EntityId entity; std::string entityId; const ComponentCodecRegistry::Entry* codec; std::uint32_t version; const Json* data; std::string pointer; };
        auto world = std::make_unique<World>();
        context.m_Entities.clear();
        std::vector<EntityId> created;
        created.reserve(entities.size());
        std::vector<PendingComponent> pending;
        std::uint64_t totalComponents = 0;

        for (std::size_t index = 0; index < entities.size(); ++index)
        {
            const auto& item = entities[index];
            const auto base = "/entities/" + std::to_string(index);
            if (!item.is_object() || !item.contains("id") || !item["id"].is_string()
                || !item.contains("components") || !item["components"].is_array())
                return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "entity requires string id and components array", base));
            const auto id = item["id"].get<std::string>();
            if (id.empty() || !context.m_Entities.emplace(id, entt::null).second)
                return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "empty or duplicate entity id", base + "/id"));
            if (item["components"].size() > context.m_Limits.maxComponentsPerEntity)
                return std::unexpected(Error(ArchiveErrorCode::ResourceLimitExceeded, "components per entity exceed archive limit", base + "/components"));
            if (item["components"].size() > context.m_Limits.maxComponents
                || totalComponents > context.m_Limits.maxComponents - item["components"].size())
                return std::unexpected(Error(ArchiveErrorCode::ResourceLimitExceeded, "component count exceeds archive limit", base + "/components"));
            totalComponents += item["components"].size();

            auto entity = world->CreateEntity();
            context.m_Entities[id] = entity;
            created.push_back(entity);
            std::set<std::string> seenTypes;
            for (std::size_t c = 0; c < item["components"].size(); ++c)
            {
                const auto& component = item["components"][c];
                const auto pointer = base + "/components/" + std::to_string(c);
                if (!component.is_object() || !component.contains("type") || !component["type"].is_string()
                    || !component.contains("version") || !component.contains("data"))
                    return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "component requires type, version, and data", pointer));
                const auto type = component["type"].get<std::string>();
                if (!seenTypes.emplace(type).second)
                    return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "duplicate component type on entity", pointer + "/type"));
                const auto* codec = codecs.Find(type);
                if (!codec)
                    return std::unexpected(Error(ArchiveErrorCode::UnknownComponent, "unknown component type '" + type + "'", pointer + "/type"));
                if (codec->transient)
                    return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "transient component cannot appear in a world document", pointer + "/type"));
                auto version = ReadVersion(component["version"], pointer + "/version");
                if (!version) return std::unexpected(version.error());
                if (*version != codec->version)
                    return std::unexpected(Error(ArchiveErrorCode::UnsupportedVersion, "unsupported component version", pointer + "/version"));
                pending.push_back({entity, id, codec, *version, &component["data"], pointer + "/data"});
            }
        }

        for (const auto& component : pending)
        {
            Serialization::Result<void> decoded;
            try
            {
                decoded = component.codec->load(*world, component.entity, *component.data, component.version, context);
            }
            catch (const std::bad_alloc&)
            {
                decoded = std::unexpected(Error(ArchiveErrorCode::AllocationFailure, "allocation failed in component codec"));
            }
            catch (const std::exception& exception)
            {
                decoded = std::unexpected(Error(ArchiveErrorCode::CodecFailure, exception.what()));
            }
            catch (...)
            {
                decoded = std::unexpected(Error(ArchiveErrorCode::CodecFailure, "component codec threw an unknown exception"));
            }
            if (!decoded)
            {
                auto error = decoded.error();
                error.entityId = component.entityId;
                error.componentType = component.codec->type;
                if (error.jsonPointer.empty()) error.jsonPointer = component.pointer;
                return std::unexpected(std::move(error));
            }
        }
        return world;
    }
    catch (const std::bad_alloc&)
    {
        return std::unexpected(Error(ArchiveErrorCode::AllocationFailure, "allocation failed while deserializing World"));
    }
    catch (const std::exception& exception)
    {
        return std::unexpected(Error(ArchiveErrorCode::CodecFailure, exception.what()));
    }
}
} // namespace Aether

namespace Aether::Serialization
{
Result<Json> SaveContext::EncodeEntity(EntityId entity) const
{
    if (entity == entt::null) return Json(nullptr);
    const auto it = m_EntityIds.find(static_cast<std::uint32_t>(entt::to_integral(entity)));
    if (it == m_EntityIds.end())
        return std::unexpected(Error(ArchiveErrorCode::InvalidArgument, "entity reference is invalid or not alive"));
    return Json(it->second);
}

Result<EntityId> LoadContext::ResolveEntity(std::string_view id) const
{
    const auto it = m_Entities.find(std::string(id));
    if (it == m_Entities.end())
        return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "entity reference does not exist"));
    return it->second;
}

Result<std::optional<EntityId>> LoadContext::ResolveEntity(const Json& id) const
{
    if (id.is_null()) return std::optional<EntityId>{};
    if (!id.is_string()) return std::unexpected(Error(ArchiveErrorCode::InvalidFormat, "entity reference must be a string or null"));
    const auto text = id.get<std::string>();
    auto entity = ResolveEntity(std::string_view(text));
    if (!entity) return std::unexpected(entity.error());
    return std::optional<EntityId>(*entity);
}

} // namespace Aether::Serialization
