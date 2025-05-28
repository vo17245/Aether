#pragma once
#include "Core/Math.h"
#include <functional>
namespace Aether::UI
{
    struct BaseComponent
    {
        Vec2f position = {0, 0};
        Vec2f size = {0, 0};
        float z=0;
        bool layoutEnabled = true; // 是否启用布局，如果为false，则不参与布局计算(和web上脱离流的元素类似)
    };
}