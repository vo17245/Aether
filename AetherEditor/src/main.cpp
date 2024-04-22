#include "Layers/EditorLayer.h"
#include "Editor.h"
using namespace Aether;
using namespace Editor;

int main()
{
	Application::Init();
	::Aether::Editor::Editor::Get().Run();
	int ret=Application::Get().Run();
	Application::Release();
	return ret;
}