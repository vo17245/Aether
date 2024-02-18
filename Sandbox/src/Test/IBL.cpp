#include "IBL.h"
#include "Aether/Core/Assert.h"
#include "Aether/Core/Math.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Core/Application.h"
#include "Aether/Resource/Image.h"

namespace Aether
{
    namespace Test
    {
        REGISTER_TEST(IBL);
        IBL::IBL()
            :m_Controller(3.1415926535 / 4, 0.1, 1000, 1)
        {
            //set aspect ratio
            auto window_size = Application::Get().GetWindowSize();
            Real aspectRatio = Real(window_size.x()) / Real(window_size.y());
            m_Controller.GetCamera().SetAspectRatio(aspectRatio);
            //load box
            {
                 auto model=ModelLoader::LoadFromFile("../../Asset/Model/cube.glb");
                m_SC.model=model;
                m_SC.model->Bind();
            }
            //skybox shader
            {
                auto opt_src=ShaderSource::LoadFromFile("../../Aether/shader/skybox_vs.glsl", 
                "../../Aether/shader/skybox_fs.glsl");
                AETHER_ASSERT(opt_src&&"Failed to load shader source");
                auto shader=Shader::Create(*opt_src.value());
                AETHER_ASSERT(shader&&"Failed to create shader");
                 m_SC.shader=shader;    
            }
            LoadCubeMap();
            //model
            {
                auto model = ModelLoader::LoadFromFile(
                "../../Asset/Model/sphere.fbx");
                AETHER_ASSERT(model&&"Failed to load model");
                m_Model=model;
                m_Model->Bind();
            }
            //pbr shader
            {
                auto opt_src=ShaderSource::LoadFromFile("../../Aether/shader/pbr_vs.glsl",
                "../../Aether/shader/pbr_fs.glsl");
                opt_src.value()->AddFragmentShaderMacro("IBL");
                AETHER_ASSERT(opt_src&&"Failed to load shader source");
                auto shader=Shader::Create(*opt_src.value());
                AETHER_ASSERT(shader&&"Failed to create shader");
                m_Shader=shader;
            }
            //for skybox rendering
            glDepthFunc(GL_LEQUAL);
            //ibl tex
            LoadIBLDiffuseCubeMap();
            LoadIBLSpecularCubeMap();
            LoadIBLBrdfLUT();
        }
        IBL::~IBL()
        {
        }
        void IBL::OnRender()
        {
            RenderSkybox();
            RenderModel();
        }
        bool IBL::OnWindowResize(WindowResizeEvent& e)
        {
            Real aspectRatio = Real(e.GetWidth()) / e.GetHeight();
            m_Controller.GetCamera().SetAspectRatio(aspectRatio);
            OpenGLApi::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
            return true;
        }
        Ref<CubeMap> IBL::LoadCubeMapInDir(const std::string& dir)
        {
            auto px = Image::LoadFromFileDataFormat2Float32(dir + "/px.hdr");
            auto nx = Image::LoadFromFileDataFormat2Float32(dir + "/nx.hdr");
            auto py = Image::LoadFromFileDataFormat2Float32(dir + "/ny.hdr");
            auto ny = Image::LoadFromFileDataFormat2Float32(dir + "/py.hdr");
            auto pz = Image::LoadFromFileDataFormat2Float32(dir + "/pz.hdr");
            auto nz = Image::LoadFromFileDataFormat2Float32(dir + "/nz.hdr");
            Ref<CubeMap> cubemap = CubeMap::Create(px.value(), nx.value(),
                py.value(), ny.value(),
                pz.value(), nz.value());
            return cubemap;
        }
        bool IBL::LoadCubeMap()
        {
            m_SC.cubeMap = LoadCubeMapInDir("../../Asset/Texture/skybox/hdr");
            //m_SC.cubeMap =LoadCubeMapInDir(
            //    "../../Asset/Texture/skybox/hdr/ibl/DiffuseHDR_CubeMap");
            return true;
        }
        void IBL::RenderSkybox()
        {
            m_SC.cubeMap->Bind(0);
            m_SC.shader->Bind();
            auto& camera = m_Controller.GetCamera();
            camera.CalculateProjection();
            camera.CalculateView();
            Mat4 view = camera.GetView();
            Mat4 proj = camera.GetProjection();
            m_SC.shader->SetMat4f("u_View", view);
            m_SC.shader->SetMat4f("u_Projection", proj);
            m_SC.shader->SetInt("u_EnvMap", 0);
            m_SC.model->Render();
        }
        void IBL::UpdateUniform()
        {
            m_Shader->Bind();
            m_Shader->SetVec3f("u_Albedo", m_Albedo);
            m_Shader->SetFloat("u_Ao", m_Ao);
            for (size_t i = 0;i < 4;i++)
            {
                std::string pos_label = fmt::format("u_LightPositions[{}]", i);
                m_Shader->SetVec3f(pos_label, m_LightPos[i]);
                std::string color_label = fmt::format("u_LightColors[{}]", i);
                m_Shader->SetVec3f(color_label, m_LightColor[i]);
            }
            m_Shader->SetInt("u_LightCnt", 4);
        }
        void IBL::RenderModel()
        {
            m_Shader->Bind();
            UpdateUniform();
            auto& camera = m_Controller.GetCamera();
            camera.CalculateProjection();
            camera.CalculateView();
            Vec3 cameraPos = camera.GetPosition();
            m_Shader->SetVec3f("u_CamPos", cameraPos);
            m_Shader->SetVec3f("u_Albedo", m_Albedo);
            m_Shader->SetFloat("u_Ao", m_Ao);
            Mat4 modelMatrix = Mat4::Identity();
            size_t x_cnt = 7;
            size_t y_cnt = 7;
            Real metallic_delta = 1.f / (x_cnt + 1);
            Real roughness_delta = 1.f / (y_cnt + 1);
            Real metallic = 1- metallic_delta;
            Real roughness = roughness_delta;
            for (size_t x = 0;x < x_cnt;x++)
            {
                for (size_t y = 0;y < y_cnt;y++)
                {
                    modelMatrix = Transform::Translation(Vec3(x * 3, y * 3, -10));
                    DrawModel(modelMatrix, camera.GetView(), camera.GetProjection(),
                        roughness, metallic);
                    metallic -= metallic_delta;
                    metallic = std::max(0.f, metallic);
                }
                roughness += roughness_delta;
                roughness = std::min(1.f, roughness);
            }
        }
        void IBL::DrawModel(
            const Mat4& modelMatrix, const Mat4& viewMatrix, const Mat4& projectionMatrix,
            float roughness, float metallic)
        {
            
            m_Shader->Bind();
            m_DiffuseCubeMap->Bind(0);
            m_SpecularCubeMap->Bind(1);
            m_BrfdLUT->Bind(2);
            m_Shader->SetInt("u_PrefilterMap", 1);
            m_Shader->SetInt("u_BrdfLUT", 2);
            m_Shader->SetInt("u_IBL_DiffuseMap",0);
            m_Shader->SetMat4f("u_Model", modelMatrix);
            m_Shader->SetFloat("u_Roughness", roughness);
            m_Shader->SetFloat("u_Metallic", metallic);
            Mat4 normalMatrix = modelMatrix.inverse();//rendering in world space
            normalMatrix.transposeInPlace();
            m_Shader->SetMat4f("u_NormalMatrix", normalMatrix);
            Mat4 modelView = viewMatrix * modelMatrix;
            Mat4 mvp = projectionMatrix * modelView;
            m_Shader->SetMat4f("u_ModelViewProjection", mvp);
            //m_Shader->SetMat4f("u_ModelView", modelView);
            m_Model->Render();
        }
        void IBL::OnUpdate(float sec)
        {
            m_Controller.OnUpdate(sec);

        }
        void IBL::OnEvent(Event& e)
        {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<WindowResizeEvent>(AETHER_BIND_FN(OnWindowResize));
        }
        void IBL::OnImGuiRender()
        {
            Vec3 face = m_Controller.GetCamera().GetFace();
            face.normalize();
            Mat3 rotation = Transform::GetRotation(Vec3(0, 0, -1), face);
            Vec3 z_hat = -face;
            z_hat.normalize();
            Vec3 x_hat = rotation * Vec3(1, 0, 0);
            Vec3 y_hat = (z_hat).cross(x_hat);
            y_hat.normalize();
            x_hat = y_hat.cross(z_hat);
            x_hat.normalize();
            ImGui::Begin("IBL");
            ImGui::ColorEdit3("Albedo", &m_Albedo[0]);
            ImGui::InputFloat3("camera pos",
                &m_Controller.GetCamera().GetPosition()[0]);
            ImGui::InputFloat3("camera face",
                &m_Controller.GetCamera().GetFace()[0]);
            ImGui::InputFloat3("camera up",
                &m_Controller.GetCamera().GetUp()[0]);
            for (size_t i = 0;i < 4;i++)
            {
                std::string pos_label = fmt::format("light_{}_pos", i);
                ImGui::SliderFloat3(pos_label.c_str(), &m_LightPos[i][0], -10, 10);
                std::string color_label = fmt::format("light_{}_color", i);
                ImGui::SliderFloat3(color_label.c_str(), &m_LightColor[i][0],
                    0,600);
            }
            ImGui::End();
        }
        void IBL::LoadIBLDiffuseCubeMap()
        {
            m_DiffuseCubeMap = LoadCubeMapInDir(
                "../../Asset/Texture/skybox/hdr/ibl/pmrem");
            AETHER_ASSERT(m_DiffuseCubeMap && "failed to load ibl diffuse");
        }
        void IBL::LoadIBLSpecularCubeMap()
        {
            m_SpecularCubeMap=LoadCubeMapInDir(
                "../../Asset/Texture/skybox/hdr/ibl/iem");
            AETHER_ASSERT(m_SpecularCubeMap&&"failed to load ibl specular");
        }
        void IBL::LoadIBLBrdfLUT()
        {
            auto opt_img=Image::LoadFromFileDataFormat2Float32(
                "../../Asset/Texture/skybox/hdr/ibl/Brdf.hdr");
            AETHER_ASSERT(opt_img&&"failed to load brdf lut");
            m_BrfdLUT=Texture2D::Create(opt_img.value());
            AETHER_ASSERT(m_BrfdLUT&&"failed to create brdf lut");
        }
    }//namespace Test
}//namespace Aether