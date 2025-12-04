#pragma once
#include <cstdint>
namespace Aether::UI 
{
    struct AutoResizeComponent
    {
        // relative to parent size [0-1]
        float width=0;
        float height=0;
        uint8_t flag=0;
        bool ResizeWidth()
        {
            return flag&1;
        }
        bool ResizeHeight()
        {
            return flag&2;
        }
        void EnableWidth()
        {
            flag |= 1;
        }
        void EnableHeight()
        {
            flag |= 2;
        }
        void DisableWidth()
        {
            flag &= ~1;
        }
        void DisableHeight()
        {
            flag &= ~2;
        }
    };
}