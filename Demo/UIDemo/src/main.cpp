#include "TestLayer.h"
#include "Entry/Application.h"
#include "ApplicationResource.h"
using namespace Aether;

class UIDemo : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        ApplicationResource::Init(window.GetSize());
        m_Layer = new TestLayer();
        window.PushLayer(m_Layer);
    }
    virtual void OnShutdown()override
    {
        delete m_Layer;
        ApplicationResource::Destroy();
    }
    virtual void OnFrameBegin()override
    {
        ApplicationResource::s_Instance->renderResource.m_DescriptorPool->Clear();

    }
    virtual const char* GetName() const override
    {
        return "UIDemo";
    }
private:
    TestLayer* m_Layer;
};
DEFINE_APPLICATION(UIDemo);
