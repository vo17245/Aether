#pragma once
#include "../Core/Core.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Light.h"
#include <vector>
#include "../Core/Math.h"
#include "../Resource/Model/Model.h"
namespace Aether
{
	class Renderer3D
	{
	private:
		static const Camera* s_Camera;
	public:
		static void BeginScene(const Camera* camera);
		static void Submit(Ref<Model> model,const Mat4& modelMatrix);
		static void EndScene();

	};
}

