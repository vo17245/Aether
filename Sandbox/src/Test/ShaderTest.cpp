#include "ShaderTest.h"
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Asset/ModelAssetImporter.h"
#include "Aether/Render/Math.h"
#include "Aether/Core/Config.h"
#include "Aether/Render/Renderer3D.h"
#include <iostream>
Test::ShaderTest::ShaderTest()
	:m_Camera(45,0.01,1000,16.0/9)
{

	std::filesystem::path resDir(Aether::Config::ResourcePath);
	std::filesystem::path modelPath("Model/cube.glb");
	std::filesystem::path shaderPath("Shader/Basic.shader");
	auto res=Aether::ModelAssetImporter::LoadFromFile((resDir / modelPath).string());
	assert(res);
	auto shaderRes = Aether::Shader::CreateRefFromFile((resDir / shaderPath).string().c_str());
	assert(shaderRes);
	m_Shader = shaderRes.shader.value();
	auto sphere=m_Scene.CreateEntity();
	auto& mc=sphere.AddComponent<Aether::MeshComponent>(res.value());
	for (auto& mesh : mc.Meshes)
	{
		mesh->GetShader() = m_Shader;
	}
	auto& tc = sphere.AddComponent<Aether::TransformComponent>();
	tc.Translation.z() = 10;
	
	
	
}

Test::ShaderTest::~ShaderTest()
{
}

void Test::ShaderTest::OnRender()
{
	
	Aether::Renderer3D::BeginScene(&m_Camera);
	auto view = m_Scene.GetAllEntitiesWith<Aether::MeshComponent>();
	for (auto& [entity, mc] : view.each())
	{
		for (auto& mesh : mc.Meshes)
		{
			Aether::Entity e = { entity,&m_Scene };
			Eigen::Matrix4f modelMatrix;
			if (e.HasComponent<Aether::TransformComponent>())
			{
				modelMatrix = e.GetComponent<Aether::TransformComponent>().Matrix;
			}
;			Aether::Renderer3D::Submit(mesh, modelMatrix);
		}
	}
	Aether::Renderer3D::EndScene();

}

void Test::ShaderTest::OnImGuiRender()
{
	ImGui::Begin("shader");
	
	
	ImGui::End();
}

void Test::ShaderTest::OnEvent(Aether::Event& event)
{
	
}

bool Test::ShaderTest::OnFileDrop(Aether::WindowFileDropEvent& event)
{
	return false;
}
void Test::ShaderTest::OnLoopEnd()
{
	
}

void Test::ShaderTest::OnUpdate(float ds)
{
	
}

