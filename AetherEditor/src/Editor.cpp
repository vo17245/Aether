#include "Editor.h"
#include "Aether/Core/Application.h"

namespace Aether
{
    namespace Editor
    {
        void Editor::Run()
        {
            m_EditorLayer=CreateRef<EditorLayer>();
            m_UILayer=CreateRef<UILayer>();
            ::Aether::Application::Get().PushLayer(
                m_EditorLayer
            );
            ::Aether::Application::Get().PushLayer(
                m_UILayer
            );
        }
    }
}