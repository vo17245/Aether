#pragma once
#include "../Core/Core.h"
#include "Camera.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
AETHER_NAMESPACE_BEGIN
class Model;
class Renderer3D
{
private:
	static const Camera* s_Camera;

public:
	static void BeginScene(const Camera* camera);
	static void Submit( VertexArray& va, IndexBuffer& ib,  Shader& shader,const Eigen::Matrix4f& modelMatrix);
	static void EndScene() {}

};
AETHER_NAMESPACE_END