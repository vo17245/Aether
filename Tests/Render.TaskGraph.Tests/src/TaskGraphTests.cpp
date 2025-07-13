#include <UI/Debug/DebugUILayer.h>
#include <Entry/Application.h>
#include "LayerStack.h"
#include "DebugUIBehavior.h"
using namespace Aether;

class TaskGraphTests : public Application
{
public:
    virtual void OnInit(Window& window)
    {
        m_Layers = CreateScope<LayerStack>();
        UI::DebugUILayer* layer = new UI::DebugUILayer(
            "Tests/Render.TaskGraph.Tests/assets/debug_ui.lua",
            CreateScope<DebugUIBehavior>(m_Layers.get()));
        window.PushLayer(layer);
        m_Layers->PushLayer(Scope<Layer>(layer));
    }
    virtual void OnShutdown()
    {
        m_Layers->Clear();
    }
    
private:
    Scope<LayerStack> m_Layers;

};
DEFINE_APPLICATION(TaskGraphTests);