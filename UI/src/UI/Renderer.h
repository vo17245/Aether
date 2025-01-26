#pragma once
#include "Render/RenderApi.h"
#include "Render/RenderApi/DevicePipeline.h"
namespace Aether::UI
{
    class Renderer
    {
    public:
    private:
        bool CreateBasicPipeline();
        DevicePipeline m_BasicPipeline;// only draw quad with color
    }; 
}