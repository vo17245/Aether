#pragma once
#include <vector>
#include <memory>
#include "Aether/Core.h"
#include "Panels/Panel.h"

namespace Aether
{
    namespace Editor
    {
        class EditorLayer:public Layer
        {
        public:
            EditorLayer();
            ~EditorLayer()=default;
            void OnImGuiRender()override; 
            void OnRender()override;
        private:
            std::vector<std::unique_ptr<Panel>> m_Panels;
            
        };
    }
}
