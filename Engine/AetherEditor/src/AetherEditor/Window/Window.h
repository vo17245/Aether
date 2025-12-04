#pragma once
#include <Window/Window.h>
namespace AetherEditor::UI
{

class Window
{
public:
    Window() = default;
    virtual ~Window() = default;
    
private:
    Aether::Window* m_OsWindow=nullptr;
};
}
