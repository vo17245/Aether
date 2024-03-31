#pragma once

#include "Aether/Render/FrameBuffer.h"
#include "Aether/Scene/CameraController.h"
#include "Core/MainScene.h"
#include "Panel.h"
#include "Aether/ImGui.h"
#include "Aether/Render.h"

namespace Aether
{
    namespace Editor
    {
        class SceneViewPanel:public Panel
        {
        public:
            SceneViewPanel();
            ~SceneViewPanel()=default;
            
            void OnImGuiRender()override;
            void OnRender()override;
            void OnUpdate(Real ds)override;
            Ref<FrameBuffer> m_FrameBuffer;
        private:
            //framebuffer size
            size_t m_Width=1920;
            size_t m_Height=1080;
            PerspectiveCameraController m_CameraController;
        };
    }
}