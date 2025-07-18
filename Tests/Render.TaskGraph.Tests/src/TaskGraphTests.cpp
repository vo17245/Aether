#include <UI/Debug/DebugUILayer.h>
#include <Entry/Application.h>
#include "Pages/DebugUIBehavior.h"
#include "GlobalApplicationResource.h"
#include "Layers/TestLayer.h"
using namespace Aether;

class TaskGraphTests : public Application
{
public:
    virtual void OnInit(Window& window)
    {
        GlobalApplicationResource::Init();
        // test layer
        m_Layers.push_back(CreateScope<TestLayer>());
        window.PushLayer(m_Layers.back().get());


        TestLayer* testLayer = (TestLayer*)m_Layers.back().get();
        // debug UI layer
        //auto setTest = [testLayer](Scope<Test>&& test) -> void {
        //    testLayer->SetTest(std::move(test));
        //};
        //auto behavior = CreateScope<DebugUIBehavior>();
        //behavior->SetTestCallback(setTest);
        //m_Layers.push_back(CreateScope<UI::DebugUILayer>(
        //    "Tests/Render.TaskGraph.Tests/assets/debug_ui.lua",
        //    std::move(behavior)));
        //window.PushLayer(m_Layers.back().get());
    }
    virtual void OnShutdown()
    {
        m_Layers.clear();
        GlobalApplicationResource::Destroy();
    }

private:
    std::vector<Scope<Layer>> m_Layers;
    DeviceCommandBufferPool m_CommandBufferPool;
};
DEFINE_APPLICATION(TaskGraphTests);