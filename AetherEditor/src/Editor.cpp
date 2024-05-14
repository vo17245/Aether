#include "Editor.h"
#include "Aether/Core/Application.h"
#include "Layers/LuaLayer.h"
namespace Aether {
namespace Editor {
void Editor::Run()
{
    m_EditorLayer = CreateRef<EditorLayer>();
    ::Aether::Application::Get().PushLayer(
        m_EditorLayer);
    m_UILayer = CreateRef<UILayer>();
    ::Aether::Application::Get().PushLayer(
        m_UILayer);
    m_LuaLayer = CreateRef<LuaLayer>("../../AetherEditor/page/example.xml", "../../AetherEditor/lua/common_script_example.lua");
    ::Aether::Application::Get().PushLayer(
        m_LuaLayer);
}

}
} // namespace Aether::Editor