#include <ProjectAsset/Types.h>

#include <algorithm>

namespace Aether::ProjectAssets
{
bool IsStableIdentifier(std::string_view value) noexcept
{
    if (value.empty()) return false;
    const auto validFirst = [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); };
    if (!validFirst(value.front())) return false;
    return std::all_of(value.begin(), value.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
    });
}
}
