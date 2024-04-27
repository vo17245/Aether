#pragma once
#include "Aether/Core.h"
#include "Layers/EditorLayer.h"
#include "Layers/UILayer.h"
#include "Core/MainScene.h"
namespace Aether
{
    namespace Editor
    {
        class Editor
        {
        public:
            Editor()
            {
                MainScene::Init();
            }


            
            ~Editor()
            {
                MainScene::Release();
            }
            void Run();
            EditorLayer& GetEditorLayer() 
            { 
                return *dynamic_cast<EditorLayer*>(m_EditorLayer.get());
            }
            UILayer& GetUILayer() 
            {
                return *dynamic_cast<UILayer*>(m_UILayer.get());
            }
        private:
            Ref<Layer> m_EditorLayer;
            Ref<Layer> m_UILayer;
        };
    }
}