#include "PbrDirectLight.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Render/ShaderSource.h"
#include "Aether/Render/Transform.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/ShaderCache.h"
#include "Aether/Core/Application.h"
namespace Aether
{
    namespace Test
    {
        REGISTER_TEST(PbrDirectLight);
        PbrDirectLight::PbrDirectLight()
            :m_Controller(3.1415926535 / 4, 0.1, 1000, 1)
        {
            
            m_Controller.GetCamera().GetPosition() = Vec3(-5.814, 10.517, 46.358);
            auto window_size = Application::Get().GetWindowSize();
            Real aspectRatio = Real(window_size.x()) / Real(window_size.y());
            m_Controller.GetCamera().SetAspectRatio(aspectRatio);
            
            
            auto model = ModelLoader::LoadFromFile(
                "../../Asset/Model/sphere.fbx");
                
            AETHER_ASSERT(model&&"Failed to load model");
            m_Model=model;
            m_Model->Bind();
            
            auto opt_src=ShaderSource::LoadFromFile("../../Aether/shader/pbr_vs.glsl",
            "../../Aether/shader/pbr_fs.glsl");
            AETHER_ASSERT(opt_src&&"Failed to load shader source");
            auto shader=Shader::Create(*opt_src.value());
            AETHER_ASSERT(shader&&"Failed to create shader");
            m_Shader=shader;
            


        }
        PbrDirectLight::~PbrDirectLight()
        {
        }
       
        void PbrDirectLight::OnImGuiRender()
        {
            bool ret=false;
            ImGui::Begin("PbrTest");
            ret|=ImGui::ColorEdit3("Albedo",&m_Albedo[0]);
            ret|=ImGui::SliderFloat("ao", &m_Ao,0,1);
            for (size_t i = 0;i < 4;i++)
            {
                std::string pos_label = fmt::format("light_{}_pos", i);
                ret |= ImGui::SliderFloat3(pos_label.c_str(), &m_LightPos[i][0], -10, 10);
                std::string color_label = fmt::format("light_{}_color", i);
                ret |= ImGui::ColorEdit3(color_label.c_str(), &m_LightColor[i][0]);
            }
            ImGui::InputFloat3("camera pos", &m_Controller.GetCamera().GetPosition()[0]);
            ImGui::InputFloat3("camera face", 
                &m_Controller.GetCamera().GetFace()[0]);
            ImGui::End();
            if(ret)
            {
                UpdateUniform();
            }
        }
        void PbrDirectLight::OnRender()
        {
            m_Shader->Bind();
            UpdateUniform();
            auto& camera = m_Controller.GetCamera();
            camera.CalculateProjection();
            camera.CalculateView();
            Vec3 cameraPos = camera.GetPosition();
            m_Shader->SetVec3f("camPos", cameraPos);
            m_Shader->SetVec3f("albedo", m_Albedo);
            m_Shader->SetFloat("ao", m_Ao);
            
            Mat4 modelMatrix = Mat4::Identity();
            size_t x_cnt=7;
            size_t y_cnt=7;
            Real metallic_delta=1.f/(x_cnt-1);
            Real roughness_delta=1.f/(y_cnt-1);
            Real metallic=1;
            Real roughness=0;
            for(size_t x=0;x<x_cnt;x++)
            {
                for(size_t y=0;y<y_cnt;y++)
                {
                    modelMatrix=Transform::Translation(Vec3(x*3,y*3,-10));
                    DrawModel(modelMatrix,camera.GetView(),camera.GetProjection(),
                    roughness,metallic);
                    metallic -= metallic_delta;
                    metallic = std::max(0.f, metallic);
                }
                roughness += roughness_delta;
                roughness = std::min(1.f, roughness);
            }
            
            
            
            
        }
        void PbrDirectLight::DrawModel(
            const Mat4& modelMatrix,const Mat4& viewMatrix,const Mat4& projectionMatrix,
        float roughness,float metallic)
        {
            m_Shader->Bind();
            m_Shader->SetMat4f("u_Model",modelMatrix);
            m_Shader->SetFloat("roughness",roughness);
            m_Shader->SetFloat("metallic",metallic);
            Mat4 normalMatrix = modelMatrix.inverse();//rendering in world space
            normalMatrix.transposeInPlace();
            m_Shader->SetMat4f("u_NormalMatrix", normalMatrix);
            Mat4 modelView=viewMatrix*modelMatrix;
            Mat4 mvp = projectionMatrix * modelView;
            m_Shader->SetMat4f("u_ModelViewProjection", mvp);
            //m_Shader->SetMat4f("u_ModelView", modelView);
            m_Model->Render();
        }
    }
}