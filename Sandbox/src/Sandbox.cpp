#include "Sandbox.h"
#include "Layer/UILayer.h"
#include "Aether/Core/Application.h"
namespace Aether
{
	Sandbox::Sandbox()
	{
		Application::Get().PushLayer(CreateRef<UILayer>());
	}
}


