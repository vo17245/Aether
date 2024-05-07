#pragma once
#include "Aether/ImGui/imgui.h"
#include "Aether/Message/Message.h"
#include "Aether/Message/SubscribeReclaimer.h"
#include "Panel.h"
#include "Message/EditorMessage.h"
#include "Core/MainScene.h"
namespace Aether {
namespace Editor {
class EntityTagEditPanel : public Panel
{
public:
    EntityTagEditPanel()
    {
        m_SubscribeReclaimer
            .Subscribe<Message::EditTagEnd>(
                [this](::Aether::Message* msg) {
                    ClearBuffer();
                    OnTagEditEnd(static_cast<Message::EditTagEnd*>(msg));
                });
        m_SubscribeReclaimer.Subscribe<Message::EditTagBegin>(
            [this](::Aether::Message* msg) {
                m_ShowPanel = true;
            });
    };
    ~EntityTagEditPanel(){};
    void OnImGuiRender() override
    {
        if (!m_ShowPanel)
        {
            return;
        }
        ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                  ImGui::GetIO().DisplaySize.y * 0.5f);
        ImVec2 windowPosPivot = ImVec2(0.5f, 0.5f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always,
                                windowPosPivot);
        ImGui::SetNextWindowFocus();
        ImGui::Begin("Edit Tag");
        ImGui::InputText("Tag", m_TagBuffer, 255);
        if (ImGui::Button("save"))
        {
            auto* msg =
                new Message::EditTagEnd(m_TagBuffer);
            MessageBus::GetSingleton()
                .Publish<Message::EditTagEnd>(msg);
        }
        if (ImGui::Button("cancel"))
        {
            auto* msg =
                new Message::EditTagEnd(m_TagBuffer, true);
            MessageBus::GetSingleton()
                .Publish<Message::EditTagEnd>(msg);
        }
        ImGui::End();
    }
    void OnTagEditEnd(Message::EditTagEnd* msg)
    {
        m_ShowPanel = false;
        if (msg->canceled)
        {
            return;
        }
        auto& mainScene = MainScene::GetInstance();
        if (mainScene.HasEntitySelected())
        {
            auto& entity = mainScene.GetEntitySelected();
            if (entity.HasComponent<TagComponent>())
            {
                auto& tc = entity.GetComponent<TagComponent>();
                tc.tag = msg->tag;
            }
            else
            {
                entity.AddComponent<TagComponent>(msg->tag);
            }
        }
    }

private:
    void ClearBuffer()
    {
        memset(m_TagBuffer, 0, 256);
    }
    char m_TagBuffer[256] = {0};
    bool m_ShowPanel = false;
    SubscribeReclaimer m_SubscribeReclaimer;
};
}
} // namespace Aether::Editor