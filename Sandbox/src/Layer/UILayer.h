#pragma once

#include "Aether/Core/Layer.h"
namespace Aether
{
    class UILayer:public Layer
    {
    public:
        UILayer();
        ~UILayer()=default;
        void OnImGuiRender() override;
    };
}