#pragma once
#include "../Core/Core.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
AETHER_NAMESPACE_BEGIN
class Model;
class Renderer
{
private:
	static Camera s_Camera;

public:
	static void BeginScene(const Camera& camera);
	static void Submit(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
	static void Submit(const VertexArray& va, 
		const IndexBuffer& ib, 
		const Shader& shader,
		const Eigen::Matrix4f modelMatrix);
	static void EndScene(){}
	
};
AETHER_NAMESPACE_END