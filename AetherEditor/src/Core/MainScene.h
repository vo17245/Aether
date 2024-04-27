#pragma once
#include "Aether/Message/SubscribeReclaimer.h"
#include "Aether/Scene.h"
#include "Aether/Scene/Scene.h"
#include "Message/EditorMessage.h"
namespace Aether
{
    namespace Editor
    {
        class Editor;
        class MainScene
        {
            friend class Editor;
        public:
            ~MainScene();
            static MainScene& GetInstance()
            {
                return *s_Instance;
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
        private:
            static MainScene* s_Instance;
            static void Init()
            {
                s_Instance = new MainScene();
            }
            static void Release()
            {
                delete s_Instance;
            }
            
        };
    }
}