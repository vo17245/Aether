#pragma once
#include "Aether/Core/TypeIdProvider.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Message.h"
#include "Aether/Core.h"

namespace Aether
{
    namespace Editor
    {
        namespace UI{class UI;}
        namespace Message
        {
            struct EntitySelected:public ::Aether::Message
            {Entity entity;};
            struct SelectComponentBegin:public ::Aether::Message{};
            
            struct EditTagBegin:public ::Aether::Message{};
            struct EditTagEnd:public ::Aether::Message
            {
                EditTagEnd(const std::string& _tag,bool _canceled=false)
                    :canceled(_canceled),tag(_tag){}
                bool canceled;
                std::string tag;
            };
            struct SelectMeshFileBegin:public ::Aether::Message{};
            struct SelectLuaScriptFileBegin:public ::Aether::Message{};
            struct UILayerPop:public ::Aether::Message{UI::UI* ui;};
            struct SelectLuaCameraScriptFileBegin : public ::Aether::Message
            {};
        }
        
    }
}
