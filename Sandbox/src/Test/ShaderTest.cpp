#include "ShaderTest.h"
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Asset/ModelAssetImporter.h"

#include "Aether/Core/Config.h"
#include "Aether/Render/Renderer3D.h"
#include <iostream>
#include "Aether/Render/Light.h"
#include "Aether/Core/Assert.h"
static void AddModel(Aether::Scene& scene,Aether::Ref<Aether::Shader>& shader,std::string& path,float x)
{
	std::filesystem::path resDir("../res");
	std::filesystem::path modelPath(path);
	auto res = Aether::ModelAssetImporter::LoadFromFile((resDir / modelPath).string());
	assert(res);
	auto sphere = scene.CreateEntity();
	auto& mc = sphere.AddComponent<Aether::MeshComponent>(res.value());
	auto& tc = sphere.AddComponent<Aether::TransformComponent>();
	tc.Translation.z() = x;
	tc.CalculateMatrix();
	sphere.AddComponent<Aether::TagComponent>(path);
}
Aether::ShaderTest::ShaderTest()
	:m_Camera(45,0.01,1000,16.0/9)
{
	
	std::filesystem::path resDir("../res");
	std::filesystem::path shaderPath("Shader/Basic.shader");
	auto shaderRes = Aether::Shader::CreateRefFromFile((resDir / shaderPath).string().c_str());
	AETHER_ASSERT(shaderRes);
	m_Shader = shaderRes.shader.value();
	AddModel(m_Scene, m_Shader, std::string("Model/sibenik-cathedral-vray-fbx.glb"),10);
	
	
}

Aether::ShaderTest::~ShaderTest()
{
}

void Aether::ShaderTest::OnRender()
{
	Aether::PointLight light1(Aether::Vec3(0,0,0),Aether::Vec3(10,10,10) );
	Aether::PointLight light2(Aether::Vec3(0, 0, 0), Aether::Vec3(-10, 10, 10));
	Aether::OpenGLApi::BindFrameBuffer(0);
	Aether::Renderer3D::BeginScene(&m_Camera);
	Aether::Renderer3D::Submit(light1);
	Aether::Renderer3D::Submit(light2);
	auto view = m_Scene.GetAllEntitiesWith<Aether::MeshComponent>();
	for (auto& [entity, mc] : view.each())
	{
		for (auto& mesh : mc.Meshes)
		{
			Aether::Entity e = { entity,&m_Scene };
			Eigen::Matrix4f modelMatrix;
			if (e.HasComponent<Aether::TransformComponent>())
			{
				auto& tc = e.GetComponent<Aether::TransformComponent>();
				tc.CalculateMatrix();
				modelMatrix = tc.Matrix;
			}
;			Aether::Renderer3D::Submit(mesh,m_Shader, modelMatrix);
		}
	}
	Aether::Renderer3D::EndScene();

}

void Aether::ShaderTest::OnImGuiRender()
{
	static bool dockspaceOpen = true;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	int window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGui::Begin("Editor", &dockspaceOpen, window_flags);
	ImGui::End();
	ImGui::Begin("Scene");
	auto view=m_Scene.GetAllEntitiesWith<Aether::TagComponent>();
	for (auto& [entity,tagc] : view.each())
	{
		Aether::Entity e = { entity,&m_Scene };
		ImGui::Text("%s", tagc.Tag.c_str());
		if (e.HasComponent<Aether::TransformComponent>())
		{
			auto& tc = e.GetComponent<Aether::TransformComponent>();
			ImGui::InputFloat3((tagc.Tag+"Position").c_str(), tc.Translation.data());
		}
	}
	ImGui::End();
}

void Aether::ShaderTest::OnEvent(Aether::Event& event)
{
	
}

bool Aether::ShaderTest::OnFileDrop(Aether::WindowFileDropEvent& event)
{
	return false;
}
void Aether::ShaderTest::OnLoopEnd()
{
	
}

void Aether::ShaderTest::OnUpdate(float ds)
{
	auto view = m_Scene.GetAllEntitiesWith<Aether::TransformComponent>();
	for (auto& [entity, tc] : view.each())
	{
		tc.Translation.z() += 0.1 * ds;
	}
}

