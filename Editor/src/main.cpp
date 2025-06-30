#include "Entry/Application.h"
#include "ApplicationResource.h"
#include "Layers/MainPage.h"
#include "Layers/UpdateLayer.h"
using namespace Aether;

class Editor : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        window.SetSize(1920, 1080);
        ApplicationResource::Init(window.GetSize(),window.GetFinalTexture());
        m_MainWindow = &window;
        PushLayer<UpdateLayer>();
        PushLayer<MainPage>();
    }
    template<typename T>
    void PushLayer()
    {
        m_Layers.push_back(new T());
        m_MainWindow->PushLayer(m_Layers.back());
    }
    virtual void OnShutdown()override
    {
        for(auto* layer:m_Layers)
        {
            m_MainWindow->PopLayer(layer);
            delete layer;
        }
        ApplicationResource::Destroy();
    }
    virtual void OnFrameBegin()override
    {
        ApplicationResource::s_Instance->renderResource.m_DescriptorPool->Clear();
        ApplicationResource::s_Instance->renderer->OnFrameBegin();

    }
    virtual const char* GetName() const override
    {
        return "Editor";
    }
private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow;
};
DEFINE_APPLICATION(Editor);
