#pragma once
#include "Panel.h"
namespace Aether
{
    namespace Editor
    {
        class SelectComponentPanel:public Panel
        {
        public:
            SelectComponentPanel();
            ~SelectComponentPanel();
            void OnImGuiRender() override;
        private:
            bool m_Show = false;
        };
    }
}