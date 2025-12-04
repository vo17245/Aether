#pragma once
#include <ImGui/Core/imgui.h>
namespace ImGuiEx
{
    struct AABB
    {
        ImVec2 Min;
        ImVec2 Max;
        AABB() : Min(FLT_MAX, FLT_MAX), Max(-FLT_MAX, -FLT_MAX) {}
        AABB(const ImVec2& min, const ImVec2& max) : Min(min), Max(max) {}
        void Add(const ImVec2& point)
        {
            if (point.x < Min.x)
                Min.x = point.x;
            if (point.y < Min.y)
                Min.y = point.y;
            if (point.x > Max.x)
                Max.x = point.x;
            if (point.y > Max.y)
                Max.y = point.y;
        }
        void Add(const AABB& box)
        {
            Add(box.Min);
            Add(box.Max);
        }
    };
    


}