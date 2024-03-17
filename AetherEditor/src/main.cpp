#include "Layers/EditorLayer.h"
using namespace Aether;
using namespace Editor;

int main()
{
	Application::Init();
	Application::Get().PushLayer(CreateRef<EditorLayer>());
	int ret=Application::Get().Run();
	Application::Release();
	return ret;
}