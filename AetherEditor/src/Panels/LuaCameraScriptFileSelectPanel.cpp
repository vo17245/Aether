#include "LuaCameraScriptFileSelectPanel.h"
#include "Aether/ImGui/imgui.h"
#include "Panels/LuaCameraScriptFileSelectPanel.h"
#include "UI/WindowStyle.h"
#include "Aether/UI.h"
#include "Core/MainScene.h"
#include "Aether/Core.h"
#include "Aether/Utils/FileUtils.h"
#include "Message/EditorMessage.h"
namespace Aether {

namespace Editor {
LuaCameraScriptFileSelectPanel::LuaCameraScriptFileSelectPanel()
{
    m_Reclaimer.Subscribe<Message::SelectLuaCameraScriptFileBegin>(
        [this](Aether::Message* msg) {
            m_Show = true;
        });
}
void LuaCameraScriptFileSelectPanel::OnImGuiRender()
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
    ImGui::Begin("Select Lua Camera Script File");
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
        if (!entity.HasComponent<LuaCameraScriptComponent>())
        {
            entity.AddComponent<LuaCameraScriptComponent>();
        }
        auto& lsc = entity.GetComponent<LuaCameraScriptComponent>();
        lsc.script = std::move(script.value());
        lsc.path = m_FilePath;
        m_Show = false;
        return;
    }
}
}
} // namespace Aether::Editor