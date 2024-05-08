#pragma once
#include "Aether/ImGui.h"
namespace Aether {
namespace Editor {
class WindowStyle
{
public:
    static void SetNextModal()
    {
        ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                  ImGui::GetIO().DisplaySize.y * 0.5f);
        ImVec2 windowPosPivot = ImVec2(0.5f, 0.5f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always,
                                windowPosPivot);
        ImGui::SetNextWindowFocus();
    }
};
}
} // namespace Aether::Editor