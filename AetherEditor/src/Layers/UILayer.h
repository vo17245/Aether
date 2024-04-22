#pragma once

#include "Aether/Core.h"
#include "Aether/Core/Layer.h"
#include "Aether/Message/Message.h"
#include "Aether/Message/SubscribeReclaimer.h"
#include "UI/UI.h"
#include <vector>
#include "Message/EditorMessage.h"
#include "Aether/Message.h"
namespace Aether
{
    namespace Editor
    {
        class UILayer:public Layer
        {
        public:
            UILayer()
            {
                m_SubscribeReclaimer.Subscribe<Message::UILayerPop>([this](::Aether::Message* message)
                {
                    PopUI(dynamic_cast<Message::UILayerPop*>(message)->ui);
                });
            }
            virtual ~UILayer()=default;
            virtual void OnEvent(Event& event) 
            {
                for(auto ui:m_UI)
                {
                    ui->OnEvent(event);
                }
            }
            virtual void OnRender()
            {
                for(auto ui:m_UI)
                {
                    ui->OnRender();
                }
            }
            virtual void OnImGuiRender()
            {
                for(auto ui:m_UI)
                {
                    ui->OnImGuiRender();
                }
            }
            virtual void PushUI(UI::UI* ui)
            {
                m_UI.push_back(ui);
            }
            virtual void PopUI(UI::UI* ui)
            {
                for(auto iter=m_UI.begin();iter!=m_UI.end();)
                {
                    if(*iter==ui)
                    {
                        iter=m_UI.erase(iter);
                    }
                    else
                    {
                        ++iter;
                    }
                }
            }
        private:
            SubscribeReclaimer m_SubscribeReclaimer;
            std::vector<UI::UI*> m_UI;
        };
    }
}