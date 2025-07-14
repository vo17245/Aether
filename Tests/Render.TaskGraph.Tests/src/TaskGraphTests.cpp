#include <UI/Debug/DebugUILayer.h>
#include <Entry/Application.h>
#include "LayerStack.h"
#include "DebugUIBehavior.h"
#include "GlobalApplicationResource.h"
using namespace Aether;

class TaskGraphTests : public Application
{
public:
    virtual void OnInit(Window& window)
    {
        GlobalApplicationResource::Init();
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
        GlobalApplicationResource::Destroy();
    }
    
private:
    Scope<LayerStack> m_Layers;
    DeviceCommandBufferPool m_CommandBufferPool;
};
DEFINE_APPLICATION(TaskGraphTests);