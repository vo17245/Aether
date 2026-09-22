#include "ImGuiLayer.h"
#include <Entry/Application.h>
#include <Async/GlobalThreadPool.h>
namespace AetherEditor
{

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
    }
    virtual void OnShutdown() override
    {
        for (auto* layer : m_Layers)
        {
            if (m_MainWindow) m_MainWindow->PopLayer(layer);
            delete layer;
        }
        m_Layers.clear();
        m_MainWindow = nullptr;
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
        param.title = "AetherEditor";
        return param;
    }

private:
    std::vector<Layer*> m_Layers;
    Window* m_MainWindow = nullptr;
};
} // namespace AetherEditor

DEFINE_APPLICATION(AetherEditor::AetherEditor);
