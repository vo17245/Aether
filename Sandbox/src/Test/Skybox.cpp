#include "Skybox.h"
#include "Aether/Core/Assert.h"
#include "Aether/Core/Math.h"
#include "Aether/Resource/Model/ModelLoader.h"
namespace Aether
{
    namespace Test
    {
        REGISTER_TEST(Skybox);
        Skybox::Skybox()
            :m_Controller(3.1415926535 / 4, 0.1, 1000, 1)
        {
            auto opt_model=ModelLoader::LoadFromFile("../../Asset/Model/sphere.fbx");
            m_Model=opt_model.value();
            m_Model->Bind();
            auto opt_src=ShaderSource::LoadFromFile("../../Aether/shader/skybox_vs.glsl", 
            "../../Aether/shader/skybox_fs.glsl");
            AETHER_ASSERT(opt_src&&"Failed to load shader source");
            auto shader=Shader::Create(*opt_src.value());
            AETHER_ASSERT(shader&&"Failed to create shader");
            m_SkyboxShader=shader;
            auto image=Image::LoadFromFileDataFormat2Float32("../../Asset/Texture/skybox/hdr/kloppenheim_06_puresky_4k.hdr");
            //auto image=Image::LoadFromFileDataFormat2Float32(
            //    "../../Asset/Texture/container2.png");
            AETHER_ASSERT(image&&"Failed to load image");
            m_SkyboxTex=Texture2D::Create(image.value());
        }
        Skybox::~Skybox()
        {
        }
        void Skybox::OnRender()
        {
            m_SkyboxTex->Bind();
            m_SkyboxShader->Bind();
            auto& camera=m_Controller.GetCamera();
            camera.CalculateProjection();
            camera.CalculateView();
            Mat4 view=camera.GetView();
            Mat4 proj=camera.GetProjection();
            m_SkyboxShader->SetMat4f("u_View", view);
            m_SkyboxShader->SetMat4f("u_Projection", proj);
            m_SkyboxShader->SetInt("u_EnvMap", 0);
            m_Model->Render();
        }
    }
}