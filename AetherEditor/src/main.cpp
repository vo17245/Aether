#include "Aether/Core/Application.h"
#include "EditorLayer.h"
using namespace Aether;

int main()
{
	Application::Get().PushLayer(CreateRef<EditorLayer>());
	return Application::Get().Run();
}