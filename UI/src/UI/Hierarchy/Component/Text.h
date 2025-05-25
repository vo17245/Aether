#pragma once
#include "Text/Raster/Raster.h"
#include <string>
#include <Core/Math.h>
#include <Render/RenderApi.h>
#include <Text/Font.h>
namespace Aether::UI
{
    struct TextComponent
    {
        std::string content;//text to show
        std::string fontpath;//font file path
        Vec4f color=Vec4f(1,1,1,1);//[0-1]
        float worldSize=32;//pixel size
        DeviceBuffer vertexBuffer;
        Text::Font* font=nullptr;
        std::unique_ptr<Text::Raster::RenderPassResource> renderResource;
    };
}