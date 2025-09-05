#pragma once
#include "Material.h"
#include <Core/Core.h>
#include <Render/Render.h>
using namespace Aether;
class MaterialBallScene
{
public:
    void SetMaterial(Material* material)
    {
        m_Material = material;
        OnMaterialChange();
    }
    void ReloadShader()
    {

    }
private:
    void OnMaterialChange()
    {

    }
    Material* m_Material=nullptr;
    DevicePipeline m_Pipeline;
    DevicePipelineLayout m_PipelineLayout;
    
};