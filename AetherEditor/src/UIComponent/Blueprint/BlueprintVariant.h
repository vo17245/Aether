#pragma once
#include <Variant>
#include <string>
#include "BlueprintObject.h"
namespace Aether::ImGuiComponent
{
    using BlueprintVariant = std::variant<std::monostate, int, float, std::string, bool, BlueprintObject*>;
}