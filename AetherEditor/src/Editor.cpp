#include "Editor.h"
#include "Aether/Core/Application.h"
#include "Layers/UILayer.h"
namespace Aether {
namespace Editor {
void Editor::Run()
{
    m_EditorLayer = CreateRef<EditorLayer>();
    ::Aether::Application::Get().PushLayer(
        m_EditorLayer);

    m_LuaLayer = CreateRef<UILayer>("../../AetherEditor/page/example.xml", "../../AetherEditor/lua/common_script_example.lua");
    ::Aether::Application::Get().PushLayer(
        m_LuaLayer);
}

}
} // namespace Aether::Editor