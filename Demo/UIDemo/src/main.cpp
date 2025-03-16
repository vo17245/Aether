#include "TestLayer.h"
#include "Entry/Application.h"
using namespace Aether;

class UIDemo : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        m_Layer = new TestLayer();
        window.PushLayer(m_Layer);
    }
    virtual void OnShutdown()override
    {
        delete m_Layer;
    }
private:
    TestLayer* m_Layer;
};
DEFINE_APPLICATION(UIDemo);
