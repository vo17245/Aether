#pragma once
#include "Panel.h"
#include "Aether/Message.h"
namespace Aether
{
    namespace Editor
    {
        class EntityPanel:public Panel
        {
        public:
            EntityPanel() 
                
            {
            };
            ~EntityPanel() 
            {
               
            };
            void OnImGuiRender() override;
       
        };

    }//namespace Editor
}//namespace Aether