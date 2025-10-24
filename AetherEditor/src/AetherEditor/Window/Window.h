#pragma once
#include <Window/Window.h>
class Window
{
public:
    Window() = default;
    virtual ~Window() = default;
    
private:
    Aether::Window* m_OsWindow=nullptr;
};