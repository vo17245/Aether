#pragma once
#include "Aether/Core/Log.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Message/SubscribeReclaimer.h"
#include "Aether/Scene/Component.h"
#include "Panel.h"
#include "Aether/Message.h"
#include "Message/EditorMessage.h"
#include "Aether/UI/FileDialog.h"
#include "Core/MainScene.h"
#include "Aether/Utils/FileUtils.h"
namespace Aether {
namespace Editor {
class LuaScriptFileSelectPanel : public Panel
{
public:
    LuaScriptFileSelectPanel()
    {
        m_SubscribeReclaimer
            .Subscribe<::Aether::Editor::Message::SelectLuaScriptFileBegin>([this](::Aether::Message* msg) {
                m_Show = true;
            });
    };
    ~LuaScriptFileSelectPanel(){

    };
    void OnImGuiRender() override
    {
        if (!m_Show)
        {
            return;
        }
        ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                  ImGui::GetIO().DisplaySize.y * 0.5f);
        ImVec2 windowPosPivot = ImVec2(0.5f, 0.5f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always,
                                windowPosPivot);
        ImGui::SetNextWindowFocus();
        ImGui::Begin("Select Mesh File");
        ImGui::Text(fmt::format("File Path:{}", m_FilePath).c_str());
        bool openExplorer = ImGui::Button("open file explorer");
        bool save = ImGui::Button("save");
        bool cancel = ImGui::Button("cancel");
        ImGui::End();
        if (openExplorer)
        {
            m_FilePath = OpenFileDialog();
            return;
        }
        if (cancel)
        {
            m_FilePath = "";
            m_Show = false;
            return;
        }
        if (save)
        {
            auto& scene = MainScene::GetInstance();
            if (!scene.HasEntitySelected())
            {
                m_Show = false;

                AETHER_DEBUG_LOG_ERROR("no entity selected");
                return;
            }
            auto script = FileUtils::ReadFileAsString(m_FilePath);
            if (!script)
            {
                m_Show = false;

                AETHER_DEBUG_LOG_ERROR("read file failed");
                return;
            }
            auto& entity = scene.GetEntitySelected();
            if (!entity.HasComponent<LuaScriptComponent>())
            {
                entity.AddComponent<LuaScriptComponent>();
            }
            auto& lsc = entity.GetComponent<LuaScriptComponent>();
            lsc.script = std::move(script.value());
            lsc.path = m_FilePath;
            m_Show = false;
            return;
        }
    }

private:
    bool m_Show = false;
    std::string m_FilePath;
    SubscribeReclaimer m_SubscribeReclaimer;
};
}
} // namespace Aether::Editor