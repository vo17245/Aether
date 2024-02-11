#include "SceneLayer.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/Renderer3D.h"
#include "Aether/Render/Camera.h"
#include "Aether/ImGui/ImGui"
namespace Aether
{
    SceneLayer::SceneLayer()
        :m_Controller(3.1415926535 / 4, 0.1, 1000, 1)
    {
        std::filesystem::path cubeModelPath="../../Asset/Model/cube.glb";
        auto opt_Model=ModelLoader::LoadFromFile(cubeModelPath);
        AETHER_ASSERT(opt_Model&&"failed to load model");
        m_Model=opt_Model.value();
        m_Model->Bind();
    }
    void SceneLayer::OnRender()
    {
        m_Controller.GetCamera().CalculateProjection();
        m_Controller.GetCamera().CalculateView();
        Renderer3D::BeginScene(&m_Controller.GetCamera());
        Renderer3D::Submit(m_Model, Mat4::Identity());
        Renderer3D::EndScene();
    }
    void SceneLayer::OnUpdate(float sec)
    {
        m_Controller.OnUpdate(sec);
    }
    void SceneLayer::OnImGuiRender()
    {
        ImGui::Begin("Scene Camera");
        ImGui::InputFloat4("camera position", 
            m_Controller.GetCamera().GetPosition().data());
        ImGui::End();
    }
}