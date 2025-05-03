#pragma once
#include "Hierarchy.h"
#include <concepts>
#include <tinyxml2.h>
#include <expected>
#include <string>
namespace Aether::UI
{
template <typename T>
concept HierarchyLoaderNodeCreator = requires(T t,Hierarchy& hierarchy, const tinyxml2::XMLNode& node) {
    {t(hierarchy,node)} -> std::same_as<std::expected<Node*, std::string>>;
};
using HierarchyLoaderNodeCreatorI = std::function<std::expected<Node*, std::string>(Hierarchy&,
                                                                         const tinyxml2::XMLNode&)>;

} // namespace Aether::UI