#pragma once
#include "../Hierarchy.h"
#include <concepts>
#include <tinyxml2.h>
#include <expected>
#include <string>
#include <sol/sol.hpp>
namespace Aether::UI
{
template <typename T>
concept HierarchyXmlLoaderNodeCreator = requires(T t,Hierarchy& hierarchy, const tinyxml2::XMLNode& node) {
    {t(hierarchy,node)} -> std::same_as<std::expected<Node*, std::string>>;
};
using HierarchyXmlLoaderNodeCreatorI = std::function<std::expected<Node*, std::string>(Hierarchy&,
                                                                         const tinyxml2::XMLNode&)>;
template<typename T>
concept HierarchyLuaLoaderNodeCreator = requires(T t, Hierarchy& hierarchy, sol::table node) {
    {t(hierarchy, node)} -> std::same_as<std::expected<Node*, std::string>>;
};
using HierarchyLuaLoaderNodeCreatorI = std::function<std::expected<Node*, std::string>(Hierarchy&,
                                                                         sol::table)>;

} // namespace Aether::UI