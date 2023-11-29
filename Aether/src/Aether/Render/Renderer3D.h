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
			void Draw();
			VisualObject(Ref<Mesh>& _mesh,
				Ref<Shader>& _shader,
				const Mat4& _modelMatrix)
				:mesh(_mesh), shader(_shader), modelMatrix(_modelMatrix)
			{}
			bool SetDirectLight(std::vector<DirectLight>& lights);
			bool SetPointLight(std::vector<PointLight>& lights);
		};

	private:
		static const Camera* s_Camera;
		static std::vector<Light> s_Lights;
		static std::vector<VisualObject> s_VisualObjects;
		static std::vector<DirectLight> s_DirectLights;
		static std::vector<PointLight> s_PointLights;

		
	public:
		static void BeginScene(const Camera* camera);
		
		static void Submit(Ref<Mesh> mesh, Ref<Shader> shader, const Eigen::Matrix4f modelMatrix);
		static void Submit(const DirectLight light);
		static void Submit(const PointLight light);
		static void EndScene();

	};
}

