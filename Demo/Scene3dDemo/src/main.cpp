#include "Entry/Application.h"
#include "ApplicationResource.h"
#include "HierarchyLayer.h"
#include <print>
#include "UpdateLayer.h"
using namespace Aether;

class Scene3dDemo : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        ApplicationResource::Init(window.GetSize(),window.GetFinalTexture());
        m_MainWindow = &window;
        PushLayer<UpdateLayer>();
        PushLayer<HierarchyLayer>();
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
        return "Scene3dDemo";
    }
private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow;
};
DEFINE_APPLICATION(Scene3dDemo);
