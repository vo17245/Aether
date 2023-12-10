#pragma once
#include "../Core/Core.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Light.h"
#include <vector>
#include "../Core/Math.h"
namespace Aether
{
	class Renderer3D
	{
	private:
		static const Camera* s_Camera;
	

		
	public:
		static void BeginScene(const Camera* camera);
		
		static void EndScene();

	};
}

