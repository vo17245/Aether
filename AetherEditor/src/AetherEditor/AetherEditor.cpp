#include "ImGuiLayer.h"
#include <Entry/Application.h>
#include <Async/GlobalThreadPool.h>
#include <AetherEditor/Global/GlobalMessageBus.h>
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
        GlobalMessageBus::Initialize();
    }
    virtual void OnShutdown() override
    {
        for (auto* layer : m_Layers)
        {
            delete layer;
        }
        GlobalMessageBus::Shutdown();
    }
    virtual void OnFrameBegin() override
    {
        GlobalMessageBus::ProcessMessages();
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
    Window* m_MainWindow;
};
} // namespace AetherEditor

DEFINE_APPLICATION(AetherEditor::AetherEditor);