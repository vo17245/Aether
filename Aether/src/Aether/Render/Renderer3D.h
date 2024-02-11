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
		friend class Application;
	private:
		static const Camera* s_Camera;
		static Ref<Shader> s_BasicShader;
	public:
		static void BeginScene(const Camera* camera);
		static void EndScene();
		static void Submit(Ref<Model> model, const Mat4& modelMatrix);
	private:
		static bool Init();
	};
}

