#include "Renderer3D.h"
#include "Aether/Render/Renderer3D.h"
#include "Aether/Render/ShaderCache.h"
#include "Aether/Render/ShaderSource.h"
#include "OpenGLApi.h"
#include "Math.h"
#include "Eigen/Core"
#include <Eigen/Dense>
#include <sstream>
#include "../Core/Log.h"

namespace Aether
{

Ref<Shader> Renderer3D::s_BasicShader;
const Camera* Renderer3D::s_Camera=nullptr;


void Renderer3D::BeginScene(const Camera* camera)
{
	s_Camera = camera;
}

void Renderer3D::EndScene()
{
}

void Renderer3D::Submit(Ref<Model> model,const Mat4& modelMatrix)
{
	auto viewMatrix=s_Camera->GetView();
	auto projectionMatrix=s_Camera->GetProjection();
	auto modelView=viewMatrix*modelMatrix;
	auto mvp=projectionMatrix*modelView;
	auto normalMatrix=modelView.inverse().transpose();//rendering in camera space
	s_BasicShader->Bind();
	s_BasicShader->SetMat4f("u_NormalMatrix",normalMatrix);
	//s_BasicShader->SetMat4f("u_Model",modelMatrix);
	//s_BasicShader->SetMat4f("u_Projection", projectionMatrix);
	s_BasicShader->SetMat4f("u_ModelViewProjection", mvp);
	//s_BasicShader->SetMat4f("u_View", viewMatrix);
	s_BasicShader->SetMat4f("u_ModelView", modelView);
	model->Render();

}
bool Renderer3D::Init()
{

	s_BasicShader=ShaderCache::GetInstance().GetShader(
		BuiltinShaderSignature(BuiltinShaderType::BASIC,0));
	return true;
}

}//namespace Aether