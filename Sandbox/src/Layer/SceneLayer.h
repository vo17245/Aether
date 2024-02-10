#pragma once
#include "Aether/Core/Layer.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Resource/Model/Model.h"
namespace Aether
{
    class SceneLayer:public Layer
    {
    public:
        SceneLayer();
        ~SceneLayer() = default;
        SceneLayer(const SceneLayer&)=delete;
        SceneLayer(SceneLayer&&)=delete;
        void OnRender()override;
    private:
        Ref<Model> m_Model;
    };
}