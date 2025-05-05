#include "Entry/Application.h"
#include "ApplicationResource.h"
#include "TextLayer.h"
using namespace Aether;

class TextDemo : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        ApplicationResource::Init();
        auto* textLayer=new TextLayer();
        m_Layers.push_back(textLayer);
        window.PushLayer(textLayer);
    }
    virtual void OnShutdown()override
    {
        for(auto* layer:m_Layers)
        {
            delete layer;
        }
        ApplicationResource::Destroy();
    }
    virtual void OnFrameBegin()override
    {

    }
    virtual const char* GetName() const override
    {
        return "TextDemo";
    }
private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow;
};
DEFINE_APPLICATION(TextDemo);
