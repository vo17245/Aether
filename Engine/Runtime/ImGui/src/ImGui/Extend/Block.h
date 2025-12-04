#pragma once
#include <ImGui/Core/imgui.h>
#include "AABB.h"
namespace ImGuiEx
{
    void BeginBlock();
    AABB EndBlock();
    void BlockPushLast();
}