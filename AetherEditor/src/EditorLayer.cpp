#include "EditorLayer.h"
#include "Aether/Scene/Component.h"
#include "Aether/Asset/ModelAssetImporter.h"
#include "Aether/Core/Config.h"
#include "Aether/Scene/VisualServer.h"
Aether::EditorLayer::EditorLayer()
	: m_CameraController(45,0.01,100,1600.0/900)
{
	m_FB = Aether::CreateRef<Aether::FrameBuffer>(1600, 900);
	OpenGLApi::BindFrameBuffer(0);
	auto teapot=m_Scene.CreateEntity();
	std::string teapotModelPath = std::string(GetConfig().resource_path) + "/Model/teapot.obj";
	auto res1=ModelAssetImporter::LoadFromFile(teapotModelPath);
	
	if (!res1)
	{
		Log::Error("failed to load {}", teapotModelPath);
		return;
	}
	teapot.AddComponent<VisualComponent>(res1.value());
	teapot.AddComponent<TagComponent>("teapot");
	teapot.AddComponent<TransformComponent>();
	auto light1 = m_Scene.CreateEntity();
	light1.AddComponent<PointLightComponent>(Vec3(1, 1, 1), Vec3(100, 100, 100));
	light1.AddComponent<TagComponent>("light1");
	auto light2 = m_Scene.CreateEntity();
	light2.AddComponent<PointLightComponent>(Vec3(1, 1, 1), Vec3(-100, 100, 100));
	light2.AddComponent<TagComponent>("light2");
	auto cube= m_Scene.CreateEntity();
	std::string cubeModelPath = std::string(GetConfig().resource_path) + "/Model/cube.glb";
	auto res2 = ModelAssetImporter::LoadFromFile(cubeModelPath);
	cube.AddComponent<VisualComponent>(res2.value());
	cube.AddComponent<TagComponent>("cube");
	cube.AddComponent<TransformComponent>();
}

Aether::EditorLayer::~EditorLayer()
{

}

void Aether::EditorLayer::OnImGuiRender()
{
	
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	{
		static bool dockspaceOpen = true;
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		int window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoTitleBar;
		ImGui::Begin("Editor", &dockspaceOpen, window_flags);
		ImGui::End();
	}
	
	{
		static int window_flags = ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoTitleBar;
		static bool open=true;
		
		ImGui::Begin("Scene",&open,window_flags);
		ImGui::Text("Hello");
		auto view = m_Scene.GetAllEntitiesWith<TagComponent>();
		for (auto& [entity, tgc] : view.each())
		{
			Entity e = { entity,&m_Scene };
			if (ImGui::TreeNode(tgc.Tag.c_str()))
			{
				if (e.HasComponent<TransformComponent>())
				{
					if (ImGui::TreeNode("Transform"))
					{
						auto& tc = e.GetComponent<TransformComponent>();
						ImGui::InputFloat3("position", tc.Position.data());
						ImGui::TreePop();
					}
				}
				if (e.HasComponent<PointLightComponent>())
				{
					auto& lc = e.GetComponent<PointLightComponent>();
					
				}
				if (e.HasComponent<PointLightComponent>())
				{
					if (ImGui::TreeNode("PointLight"))
					{
						auto& lc = e.GetComponent<PointLightComponent>();
						ImGui::InputFloat3("color", lc.Light.GetColor().data());
						ImGui::InputFloat3("position", lc.Light.GetPosition().data());
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
			
		}
		
		ImGui::End();
	}
	{
		static int window_flags = ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoTitleBar;
		static bool open = true;
		ImGui::Begin("SceneView", &open, window_flags);
		ImVec2 size = ImGui::GetWindowSize();
		ImGui::Image((void*)m_FB->GetTexture()->GetRendererId(), 
			ImVec2(m_FB->GetTexture()->GetWidth(),
				m_FB->GetTexture()->GetHeight()));
		ImGui::End();
	}
	
}

void Aether::EditorLayer::OnRender()
{
	
	m_FB->Bind();
	OpenGLApi::SetViewport(0, 0, m_FB->GetTexture()->GetWidth(), m_FB->GetTexture()->GetHeight());
	OpenGLApi::SetClearColor(0.4, 0.5, 0.6, 1.0);
	OpenGLApi::ClearColorBuffer();
	OpenGLApi::ClearDepthBuffer();
	m_CameraController.GetCamera().CalculateProjection();
	m_CameraController.GetCamera().CalculateView();
	VisualServer::Render(m_Scene,m_CameraController.GetCamera());
	m_FB->Unbind();
}

void Aether::EditorLayer::OnEvent(Event& e)
{
	
}

void Aether::EditorLayer::OnUpdate(float ds)
{
	
	m_CameraController.OnUpdate(ds);
}

bool Aether::EditorLayer::OnWindowResize(WindowResizeEvent& e)
{
	return false;
}
