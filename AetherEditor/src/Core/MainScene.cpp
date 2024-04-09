#include "MainScene.h"
#include "Aether/Scene/Component.h"
#include "Aether/Message.h"

namespace Aether
{
    namespace Editor
    {
        MainScene::MainScene()
            :m_CallbackSignature(0,0)
        {
            // primary camera
            //auto pc=m_Scene.CreateEntity();
            //auto& cc=pc.AddComponent<PerspectiveCameraComponent>();
            //cc.isPrimary=true;
            m_CallbackSignature=MessageBus::GetSingleton().
            Subscribe<Message::EntitySelected>(
                [this](::Aether::Message* msg)
                {
                    this->OnEntitySelected(
                        dynamic_cast<Message::EntitySelected*>(msg));
                }
                
            );
        }
        MainScene::~MainScene()
        {
            MessageBus::GetSingleton().Unsubscribe(m_CallbackSignature);
        }
    }
}