#pragma once

#include "Core/MainScene.h"
#include "Panel.h"
#include "Aether/ImGui.h"
namespace Aether
{
    namespace Editor
    {
        class SceneViewPanel:public Panel
        {
        public:
            SceneViewPanel()=default;
            ~SceneViewPanel()=default;
            void OnImGuiRender() override
            {
                int window_flags = 0;
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse ;
                ImGui::Begin("Scene View",nullptr,window_flags);
                ImGui::End();
            }
        };
    }
}