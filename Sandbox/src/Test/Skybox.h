#pragma once
#include "Aether/Render/Camera.h"
#include "Test/Test.h"
#include "Test/TestRegister.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Render/Shader.h"
#include "Aether/Scene/CameraController.h"
#include "Aether/Core/Input.h"
#include "Aether/Core/Log.h"
namespace Aether
{
    namespace Test
    {
        class Skybox:public Test
        {
        public:
            Skybox();
            ~Skybox();
            void OnRender()override;
            void OnUpdate(float sec)
            {
                m_Controller.OnUpdate(sec);
                auto trace = Input::Get().GetMouseTrace();
                for (auto& e : trace)
                {
                    Log::Debug("{} {}", e.GetPositionX(),
                        e.GetPositionY());
                }
            }
        private:
            Ref<Model> m_Model;
            Ref<Shader> m_SkyboxShader;
            PerspectiveCameraController m_Controller;
        };
    }
}