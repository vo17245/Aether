#include "EditorLayer.h"

Aether::EditorLayer::EditorLayer()
{
	m_FB = Aether::CreateRef<Aether::FrameBuffer>(1600, 900);
	OpenGLApi::BindFrameBuffer(0);
}

void Aether::EditorLayer::OnImGuiRender()
{
	

	static bool dockspaceOpen = true;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
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
	ImGui::Begin("Scene");
	ImGui::Text("Hello");
	ImGui::End();
	DrawSceneView();
}

void Aether::EditorLayer::OnRender()
{
	m_FB->Bind();
	OpenGLApi::SetClearColor(0.4, 0.5, 0.6, 1.0);
	OpenGLApi::ClearColorBuffer();
	OpenGLApi::ClearDepthBuffer();
	m_FB->Unbind();
}

void Aether::EditorLayer::OnEvent(Event& e)
{
	//EventDispatcher dispatcher(e);
	//dispatcher.Dispatch<WindowResizeEvent>(AETHER_BIND_FN(OnWindowResize));
}

void Aether::EditorLayer::DrawSceneView()
{
	int window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;
	ImGui::Begin("SceneView");
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::Image((void*)m_FB->GetTexture()->GetRendererId(),size);
	
	OpenGLApi::SetViewport(0,0,size.x, size.y);
	ImGui::End();
}

bool Aether::EditorLayer::OnWindowResize(WindowResizeEvent& e)
{
	
	return false;
}
