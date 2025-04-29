#pragma once
#include <functional>
namespace Aether::UI
{
    struct MouseComponent
    {
        bool isHover = false;
        bool isPress=false;
        std::function<void()> onHover=[](){};
        std::function<void()> onPress=[](){};
        std::function<void()> onRelease=[](){};
    };
}