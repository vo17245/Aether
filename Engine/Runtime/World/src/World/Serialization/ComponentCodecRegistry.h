#pragma once

#include <World/Serialization/ArchiveTypes.h>
#include <World/World.h>

#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace Aether::Serialization
{
class SaveContext;
class LoadContext;

template<class T>
struct ComponentSerialization;

class ComponentCodecRegistry
{
public:
    using SaveCallable = std::function<Result<Json>(const World&, EntityId, SaveContext&)>;
    using LoadCallable = std::function<Result<void>(World&, EntityId, const Json&, std::uint32_t, LoadContext&)>;
    struct Entry
    {
        std::string type;
        std::uint32_t version = 0;
        const entt::type_info* runtimeType = nullptr;
        bool transient = false;
        SaveCallable save;
        LoadCallable load;
    };

    template<class T>
    Result<void> Register()
    {
        using Codec = ComponentSerialization<T>;
        return Register<T>(
            [](const T& value, SaveContext& context) { return Codec::Serialize(value, context); },
            [](const Json& json, std::uint32_t version, LoadContext& context) {
                return Codec::Deserialize(json, version, context);
            });
    }

    template<class T>
    Result<void> Register(std::function<Result<Json>(const T&, SaveContext&)> save,
                          std::function<Result<T>(const Json&, std::uint32_t, LoadContext&)> load)
    {
        using Codec = ComponentSerialization<T>;
        if (m_Frozen)
            return std::unexpected(RegistryError(ArchiveErrorCode::InvalidArgument, "codec registry is frozen"));
        if (Codec::Type.empty() || Codec::Version == 0)
            return std::unexpected(RegistryError(ArchiveErrorCode::InvalidArgument, "codec type and version must be non-empty/non-zero"));
        if (!save || !load)
            return std::unexpected(RegistryError(ArchiveErrorCode::InvalidArgument, "codec callables cannot be empty"));
        const auto info = entt::type_id<T>();
        if (HasRuntimeType(info) || m_Entries.contains(std::string(Codec::Type)))
            return std::unexpected(RegistryError(ArchiveErrorCode::DuplicateRegistration, "component codec type or C++ type registered more than once"));
        Entry entry;
        entry.type = std::string(Codec::Type);
        entry.version = Codec::Version;
        entry.runtimeType = &entt::type_id<T>();
        entry.save = [fn = std::move(save)](const World& world, EntityId entity, SaveContext& context) {
            return fn(world.GetComponent<T>(entity), context);
        };
        entry.load = [fn = std::move(load)](World& world, EntityId entity, const Json& json,
                                           std::uint32_t version, LoadContext& context) -> Result<void> {
            auto decoded = fn(json, version, context);
            if (!decoded) return std::unexpected(decoded.error());
            world.AddComponent<T>(entity, std::move(*decoded));
            return {};
        };
        m_Entries.emplace(entry.type, std::move(entry));
        return {};
    }

    template<class T>
    Result<void> RegisterTransient()
    {
        using Codec = ComponentSerialization<T>;
        if (m_Frozen)
            return std::unexpected(RegistryError(ArchiveErrorCode::InvalidArgument, "codec registry is frozen"));
        if (Codec::Type.empty() || Codec::Version == 0)
            return std::unexpected(RegistryError(ArchiveErrorCode::InvalidArgument, "transient type and version must be non-empty/non-zero"));
        const auto info = entt::type_id<T>();
        if (HasRuntimeType(info) || m_Entries.contains(std::string(Codec::Type)))
            return std::unexpected(RegistryError(ArchiveErrorCode::DuplicateRegistration, "component type or C++ type registered more than once"));
        Entry entry;
        entry.type = std::string(Codec::Type);
        entry.version = Codec::Version;
        entry.runtimeType = &entt::type_id<T>();
        entry.transient = true;
        m_Entries.emplace(entry.type, std::move(entry));
        return {};
    }

    const Entry* Find(std::string_view type) const noexcept;
    const Entry* Find(const entt::type_info& runtimeType) const noexcept;
    bool HasRuntimeType(const entt::type_info& runtimeType) const noexcept;
    void Freeze() const noexcept { m_Frozen = true; }
    bool Frozen() const noexcept { return m_Frozen; }
    const std::unordered_map<std::string, Entry>& Entries() const noexcept { return m_Entries; }

private:
    static ArchiveError RegistryError(ArchiveErrorCode code, std::string message)
    {
        return {code, std::move(message)};
    }
    std::unordered_map<std::string, Entry> m_Entries;
    mutable bool m_Frozen = false;
};
} // namespace Aether::Serialization
