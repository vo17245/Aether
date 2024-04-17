#pragma once
#include <vector>
#include <memory>
#include "Aether/Core.h"
#include "Aether/Message.h"
#include "Panels/Panel.h"
#include "Panels/SelectComponentPanel.h"
#include "Aether/Scene.h"
#include "Message/EditorMessage.h"
namespace Aether
{
    namespace Editor
    {
        class EditorLayer:public Layer
        {
        public:
            EditorLayer();
            ~EditorLayer();
            void OnImGuiRender()override; 
            void OnRender()override;
            void OnUpdate(float sec)override;
        private:
            std::vector<std::unique_ptr<Panel>> m_Panels;
        };
    }
}
