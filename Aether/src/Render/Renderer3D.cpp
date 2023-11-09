#include "Renderer3D.h"
#include "OpenGLApi.h"
#include "Math.h"
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
			shader.SetMat4f("u_ModelView", s_Camera->GetProjection()*s_Camera->GetView() * modelMatrix);
		}
		else if (command.compare("use_normal_matrix") == 0)
		{
			shader.SetMat4f("u_NormalMatrix",Eigen::Transpose((s_Camera->GetView() * modelMatrix).inverse()).transpose() );
		}
		
	}
	OpenGLApi::DrawElements(va, ib);
}






AETHER_NAMESPACE_END