#pragma once
#include <vector>
#include "Core/Math/Def.h"
#include "Text/Font.h"
namespace Aether::Text
{
    struct PackGlyph
    {
        float x=0;
        float y=0;
        float width=1024;
        float height=1024;
        float maxHeight=0;
        bool PushRect(float _width,float _height,/*in*/
        float& _x,float& _y/*out*/)
        {
            if(x+_width>width)
            {
                x=0;
                y+=maxHeight;
                maxHeight=0;
            }
            if(x+_width>width)
            {
                return false;
            }
            if(y+_height>height)
            {
                return false;
            }
            _x=x;
            _y=y;
            x+=_width;
            if(maxHeight<_height)
            {
                maxHeight=_height;
            }
            return true;
        }
    };
}