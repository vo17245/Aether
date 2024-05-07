#include "MainScene.h"
#include "Aether/Render/Camera.h"
#include "Aether/Scene/Component.h"
#include "Aether/Message.h"
#include "Aether/Resource.h"
#include "Aether/Scene.h"
namespace Aether
{
    namespace Editor
    {
        MainScene* MainScene::s_Instance = nullptr;
        MainScene::MainScene()
        {
            //pbr renderer
            {
                m_PbrRenderer=CreateScope<PbrRenderer>();
                // message
                m_SubscribeReclaimer.
                Subscribe<Message::EntitySelected>(
                    [this](::Aether::Message* msg)
                    {
                        this->OnEntitySelected(
                            dynamic_cast<Message::EntitySelected*>(msg));
                    }

                );
            }
            
            // primary camera entity
            {
                auto pc=m_Scene.CreateEntity();
                auto& cc=pc.    AddComponent<PerspectiveCameraComponent>(Math::PI / 4, 0.1, 1000, 1);
                pc.AddComponent<TagComponent>("PrimatryCamera");
                cc.isPrimary=true;
                
            }
            // lua
            {
                m_LuaScriptSystem=CreateScope<LuaScriptSystem>(&m_Scene);
            }
            // axis plane entity
            {
                auto planeEntity=m_Scene.CreateEntity();
                planeEntity.AddComponent<PlaneComponent>();
                m_PlaneSystem=CreateScope<PlaneSystem>();
                planeEntity.AddComponent<TagComponent>("AxisPlane");
            }
            
            // skybox entity
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
            
            //sphere entity
            {
                //std::string sphere_model_path = "../../Asset/Model/sphere.fbx";
                std::string sphere_model_path = "../../Asset/Model/teapot.obj";
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
        MainScene::~MainScene()
        {
            
        }
        void MainScene::OnUpdate(float sec)
        {
            m_LuaScriptSystem->OnUpdate(sec);
            //primary camera
            PerspectiveCamera* primaryCamera=nullptr;
            auto view=m_Scene.GetAllEntitiesWith<PerspectiveCameraComponent>();
            for(const auto& [entity,cc]:view.each())
            {
                if(cc.isPrimary)
                {
                    primaryCamera=&cc.camera;
                    break;
                }
            }
            if(!primaryCamera)
            {
                AETHER_DEBUG_LOG_ERROR("No primary camera found");
                return;
            }
            m_CameraController.OnUpdate(sec,*primaryCamera);
        }
        void MainScene::OnRender(float aspectRatio)
        {
            OpenGLApi::SetClearColor(1.0, 0.0, 0.0, 1.0);
            OpenGLApi::ClearColorBuffer();
            OpenGLApi::ClearDepthBuffer();
            //render scene
            //primary camera
            PerspectiveCamera* primaryCamera=nullptr;
            auto view=m_Scene.GetAllEntitiesWith<PerspectiveCameraComponent>();
            for(const auto& [entity,cc]:view.each())
            {
                if(cc.isPrimary)
                {
                    primaryCamera=&cc.camera;
                    break;
                }
            }
            if(!primaryCamera)
            {
                AETHER_DEBUG_LOG_ERROR("No primary camera found");
                return;
            }
            primaryCamera->SetAspectRatio(aspectRatio);
            // pbr objects pass
            m_PbrRenderer->Render(MainScene::GetInstance().GetScene(),*primaryCamera);
            // axis plane
            m_PlaneSystem->OnRender(MainScene::GetInstance().GetScene());
        }
    }//namesapce Editor
}//namespace Aether