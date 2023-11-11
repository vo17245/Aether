#include "FrameBufferTest.h"
#include "Aether/Render/Renderer3D.h"
#include "Aether/Core/Config.h"
#include "Aether/Render/Transform.h"
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Asset/ModelAssetImporter.h"
static void AddModel(Aether::Scene& scene,
	Aether::Ref<Aether::Shader>& shader, std::string& path, float x)
{
	std::filesystem::path resDir(Aether::Config::ResourcePath);
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
Test::FrameBufferTest::FrameBufferTest()
	:m_OrthographicCamera(-800,800,-450,450,1,-10), m_Camera(45, 0.01, 1000, 16.0 / 9)
{
	m_FB=Aether::CreateRef<Aether::FrameBuffer>(1600,900);
	m_FB->Bind();
	m_Rectangle = Aether::Rectangle().GetMesh();
	auto res = Aether::Shader::CreateRefFromFile("../../Resource/Shader/FullScreenRectangle.shader");
	assert(res);
	m_ScreenShader = res.shader.value();

	std::filesystem::path resDir(Aether::Config::ResourcePath);
	std::filesystem::path shaderPath("Shader/Basic.shader");
	auto shaderRes = Aether::Shader::CreateRefFromFile((resDir / shaderPath).string().c_str());
	assert(shaderRes);
	m_Shader = shaderRes.shader.value();
	AddModel(m_Scene, m_Shader, std::string("Model/sibenik-cathedral-vray-fbx.glb"), 10);
	auto imageRes = Aether::Image::LoadFromFileDataFormat("../../Resource/Texture/container2.png");
	assert(imageRes);
	m_Texture = Aether::CreateRef<Aether::Texture2D>(imageRes.value());
}

Test::FrameBufferTest::~FrameBufferTest()
{
}

void Test::FrameBufferTest::OnRender()
{
	//Render framebuffer 
	m_FB->Bind();
	
	Aether::OpenGLApi::SetClearColor(1, 0, 0, 1);
	Aether::OpenGLApi::ClearColorBuffer();
	Aether::OpenGLApi::ClearDepthBuffer();
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
				auto& tc = e.GetComponent<Aether::TransformComponent>();
				tc.CalculateMatrix();
				modelMatrix = tc.Matrix;
			}
			Aether::Renderer3D::Submit(mesh, m_Shader, modelMatrix);
		}
	}
	Aether::Renderer3D::EndScene();
	
	//render screen
	Aether::OpenGLApi::BindFrameBuffer(0);
	Aether::OpenGLApi::SetClearColor(0.4, 0.5, 0.6, 1);
	Aether::OpenGLApi::ClearColorBuffer();
	Aether::OpenGLApi::ClearDepthBuffer();
	Aether::Renderer3D::BeginScene(&m_OrthographicCamera);
	m_ScreenShader->Bind();
	m_FB->GetTexture()->Bind();
	
	m_ScreenShader->SetInt("u_Texture", 0);
	Aether::Renderer3D::Submit(m_Rectangle, m_ScreenShader, 
		Aether::Transform::Scale(Eigen::Vector3f(800, 450, 1)));
	Aether::Renderer3D::EndScene();
}

void Test::FrameBufferTest::OnImGuiRender()
{
	ImGui::Begin("Scene");
	auto view = m_Scene.GetAllEntitiesWith<Aether::TagComponent>();
	for (auto& [entity, tagc] : view.each())
	{
		Aether::Entity e = { entity,&m_Scene };
		ImGui::Text("%s", tagc.Tag.c_str());
		if (e.HasComponent<Aether::TransformComponent>())
		{
			auto& tc = e.GetComponent<Aether::TransformComponent>();
			ImGui::InputFloat3((tagc.Tag + "Position").c_str(), tc.Translation.data());
		}
	}
	ImGui::End();
}
