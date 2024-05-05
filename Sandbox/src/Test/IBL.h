#pragma once
#include "Aether/Render/Camera.h"
#include "Test/Skybox.h"
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
#include "Aether/Render/Transform.h"
#include "Aether/Core/Assert.h"

namespace Aether
{
    namespace Test
    {
        struct SkyboxComponent
        {
            Ref<Model> model;
            Ref<Shader> shader;
            Ref<CubeMap> cubeMap;
        };
        class IBL:public Test
        {
        public:
            IBL();
            ~IBL();
            void OnRender()override;
            void OnUpdate(float sec);
            
            void OnEvent(Event& e)override;
            
            void OnImGuiRender()override;
            
        private:
            SkyboxComponent m_SC;
            Ref<Model> m_Model;
            Ref<Shader> m_Shader;
            PerspectiveCameraController m_Controller;
            PerspectiveCamera m_Camera;
            Vec3 m_Albedo=Vec3(1.0,0,0);
            Real m_Ao=1.0;
            Vec3 m_LightPos[4] = { 
                Vec3(-10,-10,10),Vec3(10,-10,10) ,Vec3(-10,10,10) ,Vec3(10,10,10) };
            //Vec3 m_LightColor[4]= { 
            //    Vec3(300,300,300),Vec3(300,300,300) 
            //    ,Vec3(300,300,300) ,Vec3(300,300,300)};
            Vec3 m_LightColor[4] = {
                Vec3(0,0,0),Vec3(0,0,0) ,
                Vec3(0,0,0) ,Vec3(0,0,0) };
            bool OnWindowResize(WindowResizeEvent& e);
            Ref<CubeMap> LoadCubeMapInDir(const std::string& dir);
            bool LoadCubeMap();
            void RenderSkybox();
            void UpdateUniform();
            void RenderModel();
            void DrawModel(
                const Mat4& modelMatrix, const Mat4& viewMatrix,
                const Mat4& projectionMatrix,
                float roughness, float metallic);
        private:
            Ref<CubeMap> m_DiffuseCubeMap;
            Ref<Texture2D> m_BrfdLUT;
            Ref<CubeMap> m_SpecularCubeMap;
            void LoadIBLDiffuseCubeMap();
            void LoadIBLSpecularCubeMap();
            void LoadIBLBrdfLUT();
            
           
        };
    }
}