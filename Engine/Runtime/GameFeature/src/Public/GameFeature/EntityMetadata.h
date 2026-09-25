#pragma once

#include <GameFeature/Types.h>
#include <World/Serialization/ComponentCodecRegistry.h>

namespace Aether::GameFeatures
{
struct PersistentEntityIdComponent { PersistentEntityId value; };
struct EntityNameComponent { std::string value; };
struct ParentComponent { PersistentEntityId parent; };
Serialization::Result<void> RegisterEntityMetadata(Serialization::ComponentCodecRegistry& codecs);
}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<GameFeatures::PersistentEntityIdComponent>
{
    static constexpr std::string_view Type = "aether.persistent-entity-id";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const GameFeatures::PersistentEntityIdComponent& component, SaveContext&)
    {
        if (!component.value.IsValid())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "persistent entity ID is invalid"});
        return Json(component.value.ToString());
    }
    static Result<GameFeatures::PersistentEntityIdComponent> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != Version || !json.is_string())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "persistent entity ID must be a UUID string"});
        auto id = GameFeatures::PersistentEntityId::Parse(json.get<std::string>());
        if (!id) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "persistent entity ID is not canonical"});
        return GameFeatures::PersistentEntityIdComponent{*id};
    }
};

template<> struct ComponentSerialization<GameFeatures::EntityNameComponent>
{
    static constexpr std::string_view Type = "aether.entity-name";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const GameFeatures::EntityNameComponent& component, SaveContext&) { return Json(component.value); }
    static Result<GameFeatures::EntityNameComponent> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != Version || !json.is_string())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "entity name must be a string"});
        return GameFeatures::EntityNameComponent{json.get<std::string>()};
    }
};

template<> struct ComponentSerialization<GameFeatures::ParentComponent>
{
    static constexpr std::string_view Type = "aether.parent";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const GameFeatures::ParentComponent& component, SaveContext&)
    {
        if (!component.parent.IsValid())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "parent entity ID is invalid"});
        return Json(component.parent.ToString());
    }
    static Result<GameFeatures::ParentComponent> Deserialize(const Json& json, std::uint32_t version, LoadContext&)
    {
        if (version != Version || !json.is_string())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "parent must be a UUID string"});
        auto id = GameFeatures::PersistentEntityId::Parse(json.get<std::string>());
        if (!id) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "parent ID is not canonical"});
        return GameFeatures::ParentComponent{*id};
    }
};

template<> struct ComponentSerialization<ExcludeFromArchiveComponent>
{
    static constexpr std::string_view Type = "aether.exclude-from-archive";
    static constexpr std::uint32_t Version = 1;
};
}
