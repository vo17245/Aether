#include "PbrTest.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Render/ShaderSource.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/ShaderCache.h"

namespace Aether
{
    namespace Test
    {
        REGISTER_TEST(PbrTest);
        PbrTest::PbrTest()
            :m_Controller(3.1415926535 / 4, 0.1, 1000, 1)
        {
            

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
        PbrTest::~PbrTest()
        {

        }
       
        void PbrTest::OnImGuiRender()
        {
            bool ret=false;
            ImGui::Begin("PbrTest");
            ret|=ImGui::ColorEdit3("Albedo",&m_Albedo[0]);
            ret|=ImGui::SliderFloat("metallic", &m_Metallic,0,1);
            ret|=ImGui::SliderFloat("roughness", &m_Roughness,0,1);
            ret|=ImGui::SliderFloat("ao", &m_Ao,0,1);
            for (size_t i = 0;i < 4;i++)
            {
                std::string pos_label = fmt::format("light_{}_pos", i);
                ret |= ImGui::SliderFloat3(pos_label.c_str(), &m_LightPos[i][0], -10, 10);
                std::string color_label = fmt::format("light_{}_color", i);
                ret |= ImGui::ColorEdit3(color_label.c_str(), &m_LightColor[i][0]);
            }
            ImGui::InputFloat3("camera pos", &m_Controller.GetCamera().GetPosition()[0]);
            ImGui::End();
            if(ret)
            {
                Log::Debug("update");
                UpdateUniform();
            }
        }
        void PbrTest::OnRender()
        {
            Mat4 modelMatrix = Mat4::Identity();
            
            auto& camera = m_Controller.GetCamera();
            camera.CalculateProjection();
            camera.CalculateView();
            m_Shader->Bind();
            Mat4 viewMatrix = camera.GetView();
            Mat4 projectionMatrix = camera.GetProjection();
            Mat4 modelView = viewMatrix * modelMatrix;
            Mat4 mvp = projectionMatrix * modelView;
            Mat4 normalMatrix = modelMatrix.inverse();//rendering in world space
            normalMatrix.transposeInPlace();
            m_Shader->SetMat4f("u_NormalMatrix", normalMatrix);
            m_Shader->SetMat4f("u_Model",modelMatrix);
            //m_Shader->SetMat4f("u_Projection", projectionMatrix);
            m_Shader->SetMat4f("u_ModelViewProjection", mvp);
            //m_Shader->SetMat4f("u_View", viewMatrix);
            //m_Shader->SetMat4f("u_ModelView", modelView);
            
            Vec3 cameraPos = camera.GetPosition();
            m_Shader->SetVec3f("camPos", cameraPos);
            m_Model->Render();
        }
    }
}