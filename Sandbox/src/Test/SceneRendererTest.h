#pragma once
#include "Aether/Render/Camera.h"
#include "Test/Test.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Scene/SceneRenderer.h"
#include "Aether/Scene/CameraController.h"
#include "Aether/Event/WindowEvent.h"
namespace Aether
{
    namespace Test
    {
        class SceneRendererTest:public Test
        {
        public:
            SceneRendererTest();
            ~SceneRendererTest()=default;
            void OnEvent(Event& e)override;
            void OnUpdate(float sec)override;
            void OnRender()override;
        private:
            Scene m_Scene;
            PerspectiveCameraController m_CameraController;
            void CreateSkybox();
            void CreateSphere();
            bool OnWindowResize(WindowResizeEvent& e);
        };
    }
}
    