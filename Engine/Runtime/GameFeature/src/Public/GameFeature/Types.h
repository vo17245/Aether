#pragma once

#include <ProjectAsset/Types.h>

#include <compare>
#include <string>

namespace Aether::GameFeatures
{
template<class Tag>
class UuidId
{
public:
    UuidId() = default;
    static UuidId Create() { return UuidId(ProjectAssets::UuidId<Tag>::Create().ToString()); }
    static std::optional<UuidId> Parse(std::string_view value)
    {
        auto parsed = ProjectAssets::UuidId<Tag>::Parse(value);
        if (!parsed) return std::nullopt;
        return UuidId(parsed->ToString());
    }
    const std::string& ToString() const noexcept { return m_Value; }
    bool IsValid() const noexcept { return !m_Value.empty(); }
    friend bool operator==(const UuidId&, const UuidId&) = default;
    friend auto operator<=>(const UuidId&, const UuidId&) = default;
private:
    explicit UuidId(std::string value) : m_Value(std::move(value)) {}
    std::string m_Value;
};

struct DocumentIdTag;
struct WorldInstanceIdTag;
struct PersistentEntityIdTag;
using DocumentId = UuidId<DocumentIdTag>;
using WorldInstanceId = UuidId<WorldInstanceIdTag>;
using PersistentEntityId = UuidId<PersistentEntityIdTag>;
using ComponentTypeId = std::string;
using FeatureId = ProjectAssets::FeatureId;
}
