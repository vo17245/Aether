#include "Renderer3D.h"
#include "OpenGLApi.h"
#include "Math.h"
#include "Eigen/Core"
#include <Eigen/Dense>
AETHER_NAMESPACE_BEGIN
const Camera* Renderer3D::s_Camera=nullptr;

void Renderer3D::BeginScene(const Camera* camera)
{
	s_Camera = camera;
}

void Renderer3D::Submit( VertexArray& va,  IndexBuffer& ib, Shader& shader, const Eigen::Matrix4f& modelMatrix)
{
	va.Bind();
	ib.Bind();
	shader.Bind();
	for (auto& command : shader.GetCommands())
	{
		if (command.compare("use_model")==0)
		{
			shader.SetMat4f("u_Model", modelMatrix);
		}
		else if (command.compare("use_view") == 0)
		{
			shader.SetMat4f("u_View", s_Camera->GetView());
		}
		else if (command.compare("use_projection") == 0)
		{
			shader.SetMat4f("u_Projection", s_Camera->GetProjection());
		}
		else if (command.compare("use_model_view") == 0)
		{
			shader.SetMat4f("u_ModelView", s_Camera->GetView() * modelMatrix);
		}
		else if (command.compare("use_model_view_projection") == 0)
		{
			shader.SetMat4f("u_ModelViewProjection", s_Camera->GetProjection()*s_Camera->GetView() * modelMatrix);
		}
		else if (command.compare("use_normal_matrix") == 0)
		{
			Eigen::Matrix4f m1 = s_Camera->GetView() * modelMatrix;
			Eigen::Matrix4f m2 = m1.inverse();
			Eigen::Matrix4f m3 = m2.transpose();
			shader.SetMat4f("u_NormalMatrix",m3 );
		}
		
	}
	OpenGLApi::DrawElements(va, ib);
}

void Renderer3D::Submit(Ref<VertexArray>& va, Ref<IndexBuffer>& ib, Ref<Shader>& shader, const Eigen::Matrix4f& modelMatrix)
{
	Submit(*va, *ib, *shader, modelMatrix);
}

void Renderer3D::Submit(Ref<Mesh>& mesh, const Eigen::Matrix4f& modelMatrix)
{
	Submit(mesh->GetVertexArray(), mesh->GetIndexBuffer(), mesh->GetShader(), modelMatrix);
}






AETHER_NAMESPACE_END