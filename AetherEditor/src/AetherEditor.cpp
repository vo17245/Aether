#include "ImGuiLayer.h"

class AetherEditor : public Application
{
public:
    virtual void OnInit(Window& window) override
    {
        ImGuiApi::EnableDocking();
        auto* imguiLayer = new ImGuiLayer();
        m_MainWindow = &window;
        m_Layers.push_back(imguiLayer);
        window.PushLayer(imguiLayer);
        FileWatcher::Start();
    }
    virtual void OnShutdown() override
    {
        for (auto* layer : m_Layers)
        {
            delete layer;
        }
    }
    virtual void OnFrameBegin() override
    {
    }
    virtual const char* GetName() const override
    {
        return "AetherEditor";
    }
    virtual WindowCreateParam MainWindowCreateParam() override
    {
        auto param = WindowCreateParam{};
        param.title="AetherEditor";
        return param;
    }

private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow;
};
DEFINE_APPLICATION(AetherEditor);