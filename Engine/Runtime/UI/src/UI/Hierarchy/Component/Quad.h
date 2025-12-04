#pragma once
#include "../../Render/Quad.h"

namespace Aether::UI
{
    struct QuadComponent
    {
        QuadComponent(const Aether::UI::QuadDesc& desc) :
            quad(desc)
        {
        }
        ::Aether::UI::Quad quad;
        bool visible = true;
        std::string imagePath;
        bool isImageDirty = false; // if true,should update image in Update callback
    };
}