#pragma once

#include "Aether/Scene/Scene.h"
#include "Panel.h"
namespace Aether
{
    namespace Editor
    {
        class SceneHierarchyPanel : public Panel
        {
        public:
            SceneHierarchyPanel() = default;
            virtual ~SceneHierarchyPanel() = default;
            virtual void OnImGuiRender() override;
            virtual void OnRender() override;
            virtual void OnEvent(Event& e) override;
            virtual void OnUpdate(Real ds) override;
        };
    }
}