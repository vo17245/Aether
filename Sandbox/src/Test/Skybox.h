#pragma once
#include "Aether/Render/Camera.h"
#include "Test/Test.h"
#include "Test/TestRegister.h"
#include "Aether/Resource/Model/Model.h"
#include "Aether/Render/Shader.h"
#include "Aether/Scene/CameraController.h"
#include "Aether/Core/Input.h"
#include "Aether/Core/Log.h"
#include "Aether/Render/Texture2D.h"
#include "Aether/Event/WindowEvent.h"
#include "Aether/Render/CubeMap.h"
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
                
            }
            void OnEvent(Event& e)override
            {
                EventDispatcher dispatcher(e);
                dispatcher.Dispatch<WindowResizeEvent>(AETHER_BIND_FN(OnWindowResize));
            }
        private:
            Ref<Model> m_Model;
            Ref<Shader> m_SkyboxShader;
            PerspectiveCameraController m_Controller;
            Ref<CubeMap> m_CubeMap;
            bool OnWindowResize(WindowResizeEvent& e)
            {
                Real aspectRatio = Real(e.GetWidth()) / e.GetHeight();
                m_Controller.GetCamera().SetAspectRatio(aspectRatio);
                OpenGLApi::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
                return true;
            }
            bool LoadCubeMap()
            {
                //TODO
                // check if image is loaded successfully
                std::string dir = "../../Asset/Texture/skybox/hdr/";
                auto& px = Image::LoadFromFileDataFormat2Float32(dir + "px.hdr");
                auto& nx = Image::LoadFromFileDataFormat2Float32(dir + "nx.hdr");
                auto& py = Image::LoadFromFileDataFormat2Float32(dir + "ny.hdr");
                auto& ny = Image::LoadFromFileDataFormat2Float32(dir + "py.hdr");
                auto& pz = Image::LoadFromFileDataFormat2Float32(dir + "pz.hdr");
                auto& nz = Image::LoadFromFileDataFormat2Float32(dir + "nz.hdr");
                m_CubeMap = CubeMap::Create(px.value(), nx.value(),
                    py.value(), ny.value(), 
                    pz.value(), nz.value());
                return true;
            }
        };
    }
}