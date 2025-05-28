#pragma once

#include <UI/Hierarchy/Hierarchy.h>
#include <string>
#include <tinyxml2.h>
using namespace Aether;
UI::Node* QueryNode(UI::Hierarchy& hierarchy,tinyxml2::XMLDocument& doc,std::string_view path);
