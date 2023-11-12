#include "Renderer3D.h"
#include "OpenGLApi.h"
#include "Math.h"
#include "Eigen/Core"
#include <Eigen/Dense>
#include "ShaderLanguage.h"
#include <sstream>
#include "../Core/Log.h"
namespace Aether
{


const Camera* Renderer3D::s_Camera=nullptr;
std::vector<Light> Renderer3D::s_Lights;
std::vector<Renderer3D::VisualObject> Renderer3D::s_VisualObjects;
std::vector<DirectLight> Renderer3D::s_DirectLights;
std::vector<PointLight> Renderer3D::s_PointLights;

void Renderer3D::BeginScene(const Camera* camera)
{
	s_Camera = camera;
	
}

void Renderer3D::Submit(Ref<Mesh>& mesh, Ref<Shader>& shader,const Eigen::Matrix4f& modelMatrix)
{
	s_VisualObjects.emplace_back(mesh, shader, modelMatrix);
}

void Aether::Renderer3D::Submit(const DirectLight& light)
{
	s_DirectLights.emplace_back(light);
}

void Aether::Renderer3D::Submit(const PointLight& light)
{
	s_PointLights.emplace_back(light);
}

void Renderer3D::VisualObject::Draw()
{
	
	mesh->GetVertexArray()->Bind();
	mesh->GetIndexBuffer()->Bind();
	shader->Bind();
	for (auto& command : shader->GetCommands())
	{
		if (command.compare("use_model") == 0)
		{
			shader->SetMat4f("u_Model", modelMatrix);
		}
		else if (command.compare("use_view") == 0)
		{
			shader->SetMat4f("u_View", s_Camera->GetView());
		}
		else if (command.compare("use_projection") == 0)
		{
			shader->SetMat4f("u_Projection", s_Camera->GetProjection());
		}
		else if (command.compare("use_model_view") == 0)
		{
			shader->SetMat4f("u_ModelView", s_Camera->GetView() * modelMatrix);
		}
		else if (command.compare("use_model_view_projection") == 0)
		{
			shader->SetMat4f("u_ModelViewProjection", s_Camera->GetProjection() * s_Camera->GetView() * modelMatrix);
		}
		else if (command.compare("use_normal_matrix") == 0)
		{
			Eigen::Matrix4f m1 = s_Camera->GetView() * modelMatrix;
			Eigen::Matrix4f m2 = m1.inverse();
			Eigen::Matrix4f m3 = m2.transpose();
			shader->SetMat4f("u_NormalMatrix", m3);
		}

	}
	OpenGLApi::DrawElements(*mesh->GetVertexArray(), *mesh->GetIndexBuffer());
}

bool Renderer3D::VisualObject::SetDirectLight(std::vector<DirectLight>& lights)
{
	if (lights.size() > ShaderLanguage::max_direct_lights)
	{
		debug_log_error("lights.size() > ShaderLanguage::max_direct_lights");
		return false;
	}
	shader->Bind();
	if (shader->SetInt("u_DirectLightCnt", lights.size()))
	{
		debug_log_error("failed to set u_DirectLightCnt");
		return false;
	}
	for (size_t i = 0;i < lights.size();i++)
	{
		std::stringstream ss1;
		ss1 << "u_DirectLights[" << std::to_string(i) << "].direct";
		if (!shader->SetVec3f(ss1.str(), lights[i].GetDirect()))
		{
			debug_log_error("failed to set direct light uniform {}", ss1.str());
			return false;
		}
		std::stringstream ss2;
		ss2 << "u_DirectLights[" << std::to_string(i) << "].color";
		if (!shader->SetVec3f(ss2.str(), lights[i].GetColor()))
		{
			debug_log_error("failed to set direct light uniform {}", ss2.str());
			return false;
		}
	}
	return true;
}

bool Renderer3D::VisualObject::SetPointLight(std::vector<PointLight>& lights)
{
	if (lights.size() > ShaderLanguage::max_point_lights)
	{
		debug_log_error("lights.size() > ShaderLanguage::max_point_lights");
		return false;
	}
	shader->Bind();
	if (shader->SetInt("u_PointLightCnt", lights.size()))
	{
		debug_log_error("failed to set u_PointLightCnt");
		return false;
	}
	for (size_t i = 0;i < lights.size();i++)
	{
		std::stringstream ss1;
		ss1 << "u_PointLights[" << std::to_string(i) << "].position";
		if (!shader->SetVec3f(ss1.str(), lights[i].GetPosition()))
		{
			debug_log_error("failed to set point light uniform {}", ss1.str());
			return false;
		}
		std::stringstream ss2;
		ss2 << "u_PointLights[" << std::to_string(i) << "].color";
		if (!shader->SetVec3f(ss2.str(), lights[i].GetColor()))
		{
			debug_log_error("failed to set point light uniform {}", ss2.str());
			return false;
		}
	}
	return true;
}

void Renderer3D::EndScene()
{
	for (auto& obj : s_VisualObjects)
	{
		obj.SetDirectLight(s_DirectLights);
		obj.SetPointLight(s_PointLights);
		obj.Draw();
	}
	s_Lights.clear();
	s_VisualObjects.clear();
	s_DirectLights.clear();
	s_PointLights.clear();
}

}//namespace Aether