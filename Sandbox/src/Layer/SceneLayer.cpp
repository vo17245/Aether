#include "SceneLayer.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Core/Assert.h"
#include "Aether/Render/Renderer3D.h"
#include "Aether/Render/Camera.h"
namespace Aether
{
    SceneLayer::SceneLayer()
    {
        std::filesystem::path cubeModelPath="../../Asset/Model/cube.glb";
        auto opt_Model=ModelLoader::LoadFromFile(cubeModelPath);
        AETHER_ASSERT(opt_Model&&"failed to load model");
        m_Model=opt_Model.value();
    }
    void SceneLayer::OnRender()
    {
        auto camera=PerspectiveCamera(3.1415926535/4,0.1,1000,1);
        Renderer3D::BeginScene(&camera);
        Renderer3D::Submit(m_Model, const Mat4 &modelMatrix)
        Renderer3D::EndScene();
    }
}