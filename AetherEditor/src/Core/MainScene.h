#pragma once
#include "Aether/Message/SubscribeReclaimer.h"
#include "Aether/Scene.h"
#include "Aether/Scene/Scene.h"
#include "Message/EditorMessage.h"
namespace Aether
{
    namespace Editor
    {
        class MainScene
        {
        public:
            ~MainScene();
            static MainScene& GetInstance()
            {
                static MainScene instance;
                return instance;
            }
            Scene& GetScene()
            {
                return m_Scene;
            }
            bool HasEntitySelected() { return bool(m_SelectedEntity); }
            Entity& GetEntitySelected() { return m_SelectedEntity.value(); }
            void OnEntitySelected(Message::EntitySelected* msg)
            {
                m_SelectedEntity = msg->entity;
            }
            void OnUpdate(float sec);
        private:
            MainScene();
            Scope<LuaScriptSystem> m_LuaScriptSystem;
            Scene m_Scene;
            std::optional<Entity> m_SelectedEntity;
            SubscribeReclaimer m_SubscribeReclaimer;
            
        };
    }
}