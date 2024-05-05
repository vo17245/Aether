#pragma once
#include "Aether/Render.h"
#include "Test/Test.h"
#include "Aether/Scene.h"
#include "Aether/Event.h"
namespace Aether
{
    namespace Test
    {
        class PbrRendererTest:public Test
        {
        public:
            PbrRendererTest();
            ~PbrRendererTest()=default;
            void OnEvent(Event& e)override;
            void OnUpdate(float sec)override;
            void OnRender()override;
        private:
            Scene m_Scene;
            PerspectiveCameraController m_CameraController;
            PerspectiveCamera m_Camera;
            void CreateSkybox();
            void CreateSphere();
            bool OnWindowResize(WindowResizeEvent& e);
            Scope<PbrRenderer> m_Renderer;
        };
    }
}
    