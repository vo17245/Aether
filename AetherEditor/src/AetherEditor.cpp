#include "AetherEditor.h"
#include "EditorLayer.h"
namespace Aether
{
	AetherEditor::AetherEditor()
	{
		PushLayer(CreateRef<EditorLayer>());
	}
}
