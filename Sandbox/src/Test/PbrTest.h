#pragma once
#include "Test.h"
#include "TestRegister.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Render/Shader.h"
#include "Aether/Message/MessageBus.h"
#include <vector>
#include "Aether/Scene/CameraController.h"
#include "Aether/Event/WindowEvent.h"
namespace Aether
{
    namespace Test
    {
        class PbrTest:public Test
        {
        private:
            void UpdateUniform()
            {
                m_Shader->Bind();
                m_Shader->SetVec3f("albedo",m_Albedo);
                m_Shader->SetFloat("metallic",m_Metallic);
                m_Shader->SetFloat("roughness",m_Roughness);
                m_Shader->SetFloat("ao",m_Ao);
                for (size_t i = 0;i < 4;i++)
                {
                    std::string pos_label = fmt::format("lightPositions[{}]", i);
                    m_Shader->SetVec3f(pos_label, m_LightPos[i]);
                    std::string color_label = fmt::format("lightColors[{}]", i);
                    m_Shader->SetVec3f(color_label,  m_LightColor[i]);
                }
            }
        public:
            PbrTest();
            ~PbrTest();
            void OnRender()override;
            void OnImGuiRender()override;
            void OnUpdate(float sec)
            {
                m_Controller.OnUpdate(sec);
            }
            void OnEvent(Event& e)override
            {
                EventDispatcher dispatcher(e);
                dispatcher.Dispatch<WindowResizeEvent>(AETHER_BIND_FN(OnWindowResize));
            }
        private:
            Ref<Model> m_Model;
            Ref<Shader> m_Shader;

            Vec3 m_Albedo=Vec3(0,0,0);
            Real m_Metallic=0;
            Real m_Roughness=0;
            Real m_Ao=0;
            Vec3 m_LightPos[4] = { Vec3::Zero(),Vec3::Zero() ,Vec3::Zero() ,Vec3::Zero() };
            Vec3 m_LightColor[4]= { Vec3::Zero(),Vec3::Zero() ,Vec3::Zero() ,Vec3::Zero() };
            PerspectiveCameraController m_Controller;
            bool OnWindowResize(WindowResizeEvent& e)
            {
                Real aspectRatio = Real(e.GetWidth()) / e.GetHeight();
                m_Controller.GetCamera().SetAspectRatio(aspectRatio);
                OpenGLApi::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
                return true;
            }
        };
    }
}