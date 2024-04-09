#pragma once
#include "Aether/Core/TypeIdProvider.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Message.h"
#include "Aether/Core.h"
namespace Aether
{
    namespace Editor
    {
        namespace Message
        {
            struct EntitySelected:public ::Aether::Message
            {Entity entity;};
            struct AddComponent:public ::Aether::Message{};
            struct ComponentSelected:public ::Aether::Message
            {
                ComponentSelected(::Aether::TypeId id)
                    :componentType(id){}
                ::Aether::TypeId componentType;
            };
            struct EditTagBegin:public ::Aether::Message{};
            struct EditTagEnd:public ::Aether::Message
            {
                EditTagEnd(const std::string& _tag,bool _canceled=false)
                    :canceled(_canceled),tag(_tag){}
                bool canceled;
                std::string tag;
            };
            struct SelectMeshFileBegin:public ::Aether::Message{};
            
        }
        
    }
}
