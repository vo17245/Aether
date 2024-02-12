#pragma once
#include "Aether/Core/Layer.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Scene/CameraController.h"
#include "Aether/Event/WindowEvent.h"
namespace Aether
{
    class SceneLayer:public Layer
    {
    public:
        SceneLayer();
        ~SceneLayer() = default;
        SceneLayer(const SceneLayer&)=delete;
        SceneLayer(SceneLayer&&)=delete;
        void OnRender()override;
        void OnUpdate(float sec)override;
        void OnImGuiRender()override;
        void OnEvent(Event& event)override;
    private:
        Ref<Model> m_Model;
        PerspectiveCameraController m_Controller;
        bool OnWindowResize(WindowResizeEvent& e);
    };
}