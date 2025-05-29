#pragma once
#include <Core/Math.h>
using namespace Aether;
struct DragComponent
{
    bool isDragging=false;
    Vec2f offset{0,0};//被选中时左上角距离鼠标的位置
};