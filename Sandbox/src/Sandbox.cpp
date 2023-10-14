#include "Sandbox.h"
#include "Test/TestMenu.h"
#include "Test/EventTest.h"
Sandbox::Sandbox()
	:m_CameraController(Aether::Camera::CreatePerspectiveCamera(16.0/9.0))
{
	auto testMenu = Aether::CreateScope<Test::TestMenu>();
	testMenu->RegisterTest("EventTest", []() {return Aether::CreateScope<Test::EventTest>();});
	m_Test = std::move(testMenu);

}

void Sandbox::OnImGuiRender()
{
	m_Test->OnImGuiRender();
	ImGui::Begin("Sandbox");
	ImGui::Text("Hello");
	ImGui::End();
}

void Sandbox::OnRender()
{
	m_Test->OnRender();
}

void Sandbox::OnEvent(Aether::Event& event)
{
	m_Test->OnEvent(event);
	m_CameraController.OnEvent(event);
}
