#pragma once
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
        float size=32;//pixel size
        DeviceBuffer vertexBuffer;
        Text::Font* font=nullptr;
    };
}