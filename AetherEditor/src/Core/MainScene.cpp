#include "MainScene.h"
#include "Aether/Scene/Component.h"
#include "Aether/Message.h"

namespace Aether
{
    namespace Editor
    {
        MainScene::MainScene()
        {
            // primary camera
            //auto pc=m_Scene.CreateEntity();
            //auto& cc=pc.AddComponent<PerspectiveCameraComponent>();
            //cc.isPrimary=true;
            m_SubscribeReclaimer.
            Subscribe<Message::EntitySelected>(
                [this](::Aether::Message* msg)
                {
                    this->OnEntitySelected(
                        dynamic_cast<Message::EntitySelected*>(msg));
                }
                
            );
            m_LuaScriptSystem=CreateScope<LuaScriptSystem>(&m_Scene);
        }
        MainScene::~MainScene()
        {
            
        }
        void MainScene::OnUpdate(float sec)
        {
            m_LuaScriptSystem->OnUpdate(sec);
        }
    }
}