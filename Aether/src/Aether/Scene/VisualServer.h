#pragma once
#include "Scene.h"
#include "../Render/Camera.h"
namespace Aether
{
	class VisualServer
	{
	public:
		static void Render(Scene& scene,Camera& camera);
	};
}
