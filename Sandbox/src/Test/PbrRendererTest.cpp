#include "Test/PbrRendererTest.h"
#include "Aether/Render/Texture2D.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Scene/Component.h"
#include "Test/TestRegister.h"
#include "Aether/Core/Application.h"
namespace Aether
{
    namespace Test
    {
        REGISTER_TEST(PbrRendererTest);
        PbrRendererTest::PbrRendererTest()
            :m_Camera(3.1415926535 / 4, 0.1, 1000, 1)
        {
            m_Renderer=CreateScope<PbrRenderer>();
            //set aspect ratio
            auto window_size = Application::Get().GetWindowSize();
            Real aspectRatio = Real(window_size.x()) / Real(window_size.y());
            m_Camera.SetAspectRatio(aspectRatio);
            CreateSkybox();
            CreateSphere();
        }
        void PbrRendererTest::CreateSkybox()
        {
            std::string env_dir="../../Asset/Texture/skybox/hdr";
            std::string irradiance_dir="../../Asset/Texture/skybox/hdr/ibl/pmrem";
            std::string prefilter_dir="../../Asset/Texture/skybox/hdr/ibl/iem";
            std::string brdf_tex_path="../../Asset/Texture/skybox/hdr/ibl/Brdf.hdr";
            std::string cube_model_path="../../Asset/Model/cube.glb";
            auto skybox=m_Scene.CreateEntity();
            auto envMap=CubeMap::LoadFromDir(env_dir);
            auto irradianceMap=CubeMap::LoadFromDir(irradiance_dir);
            auto prefilterMap=CubeMap::LoadFromDir(prefilter_dir);
            auto brdfLUT_image=
            Image::LoadFromFileDataFormat2Float32(brdf_tex_path);
            auto brdfLUT=Texture2D::Create(brdfLUT_image.value());
            auto model=ModelLoader::LoadFromFile(cube_model_path);
            AETHER_ASSERT(model && "failed to load model");
            model->Bind();
            skybox.AddComponent<SkyboxComponent>(model,
            envMap,irradianceMap,prefilterMap,brdfLUT);
        }
        void PbrRendererTest::OnEvent(Event& e)
        {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<WindowResizeEvent>(AETHER_BIND_FN(OnWindowResize));
        }
        bool PbrRendererTest::OnWindowResize(WindowResizeEvent& e)
        {
            Real aspectRatio = Real(e.GetWidth()) / e.GetHeight();
            m_Camera.SetAspectRatio(aspectRatio);
            OpenGLApi::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
            return true;
        }
        void PbrRendererTest::OnUpdate(float sec)
        {
            m_CameraController.OnUpdate(sec,m_Camera);
        }
        void PbrRendererTest::OnRender()
        {
            m_Renderer->Render(m_Scene, m_Camera);
        }
        void PbrRendererTest::CreateSphere()
        {
            std::string sphere_model_path = "../../Asset/Model/sphere.fbx";
            auto model = ModelLoader::LoadFromFile(sphere_model_path);
            model->Bind();
            auto entity = m_Scene.CreateEntity();
            entity.AddComponent<MeshComponent>(model);
            PbrMeterialComponent material;
            material.albedo = Vec3(1, 0, 0);
            material.ao = 1;
            material.metallic = 0.5;
            material.roughness = 0.5;
            entity.AddComponent<PbrMeterialComponent>(material);

        }
    }
}