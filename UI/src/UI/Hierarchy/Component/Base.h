#pragma once
#include "Core/Math.h"
#include <functional>
namespace Aether::UI
{
    struct BaseComponent
    {
        Vec2f position = {0, 0};
        Vec2f size = {0, 0};
    };
}