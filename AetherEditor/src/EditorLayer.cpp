#include "EditorLayer.h"
#include "Aether/Scene/Component.h"
#include "Aether/Asset/ModelAssetImporter.h"
#include "Aether/Core/Config.h"
#include "Aether/Scene/VisualServer.h"
Aether::EditorLayer::EditorLayer()
	:m_FbViewportSize(1600,900)
{
	m_FB = Aether::CreateRef<Aether::FrameBuffer>(1600, 900);
	OpenGLApi::BindFrameBuffer(0);
	auto entity=m_Scene.CreateEntity();
	std::string path = std::string(GetConfig().resource_path) + "/Model/teapot.obj";
	auto res=ModelAssetImporter::LoadFromFile(path);

	if (!res)
	{
		Log::Error("failed to load {}", path);
		return;
	}
	entity.AddComponent<VisualComponent>(res.value());
	entity.AddComponent<TagComponent>("teapot");
	entity.AddComponent<TransformComponent>();
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
		m_FbViewportSize.x() = size.x;
		m_FbViewportSize.y() = size.y;
		ImGui::Image((void*)m_FB->GetTexture()->GetRendererId(), size);
		ImGui::End();
	}

}

void Aether::EditorLayer::OnRender()
{
	static PerspectiveCamera sceneCamera(45, 0.01, 100, 1600.0 / 900);
	m_FB->Bind();
	OpenGLApi::SetViewport(0, 0, m_FbViewportSize.x(), m_FbViewportSize.y());
	OpenGLApi::SetClearColor(0.4, 0.5, 0.6, 1.0);
	OpenGLApi::ClearColorBuffer();
	OpenGLApi::ClearDepthBuffer();
	VisualServer::Render(m_Scene,sceneCamera);
	m_FB->Unbind();
}

void Aether::EditorLayer::OnEvent(Event& e)
{
	
}

bool Aether::EditorLayer::OnWindowResize(WindowResizeEvent& e)
{
	return false;
}
