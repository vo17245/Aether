#pragma once
#include "Aether/Message/SubscribeReclaimer.h"
#include "Panel.h"
#include "Aether/Message.h"
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
            SubscribeReclaimer m_Reclaimer;
        };
    }
}