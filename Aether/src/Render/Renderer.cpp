#include "Renderer.h"
#include "OpenGLApi.h"
#include "Math.h"
AETHER_NAMESPACE_BEGIN
Camera Renderer::s_Camera;

void Renderer::BeginScene(const Camera& camera)
{
	s_Camera = camera;
}

void Renderer::Submit(const VertexArray& va, const IndexBuffer& ib, const Shader& shader)
{
	va.Bind();
	ib.Bind();
	shader.Bind();
	Eigen::Matrix4f mvpMatrix = s_Camera.GetCameraMatrix();
	shader.SetMat4f("u_View", s_Camera.GetView());
	shader.SetMat4f("u_Projection", s_Camera.GetProjection());
	shader.SetMat4f("u_MVP", mvpMatrix);
	OpenGLApi::DrawElements(va, ib);
}

void Renderer::Submit(const VertexArray& va, const IndexBuffer& ib, const Shader& shader, const Eigen::Matrix4f modelMatrix)
{
	va.Bind();
	ib.Bind();
	shader.Bind();
	Eigen::Matrix4f mvpMatrix = s_Camera.GetCameraMatrix() * modelMatrix;
	shader.SetMat4f("u_Model", modelMatrix);
	
	shader.SetMat4f("u_View", s_Camera.GetView());
	shader.SetMat4f("u_Projection", s_Camera.GetProjection());
	shader.SetMat4f("u_MVP", mvpMatrix);
	ib.GetCount();
	OpenGLApi::DrawElements(va, ib);
}




AETHER_NAMESPACE_END