#pragma once
#include "../Core/Core.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Light.h"
#include <vector>
#include "../Core/Math.h"
namespace Aether
{
	class Renderer3D
	{

	private:
		struct VisualObject
		{
			Ref<Mesh> mesh;
			Ref<Shader> shader;
			Mat4 modelMatrix;
		};
	private:
		static const Camera* s_Camera;
		static std::vector<Light> s_Lights;
		static std::vector<VisualObject> s_VisualObjects;
	private:
		static void Submit(VertexArray& va, IndexBuffer& ib, Shader& shader, const Eigen::Matrix4f& modelMatrix);
		static void Submit(Ref<VertexArray>& va, Ref<IndexBuffer>& ib, Ref<Shader>& shader, const Eigen::Matrix4f& modelMatrix);
	public:
		static void BeginScene(const Camera* camera);
		
		static void Submit(Ref<Mesh>& mesh, Ref<Shader>& shader, const Eigen::Matrix4f& modelMatrix);
		
		static void EndScene() {}

	};
}

