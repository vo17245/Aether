#include "Sandbox.h"
#include "Test/TestMenu.h"
#include "Test/EventTest.h"
#include "Test/ShowModelTest.h"
#include "Test/FrameBufferTest.h"
#include "Test/ShaderTest.h"
Sandbox::Sandbox()
	
{
	auto testMenu = Aether::CreateScope<Test::TestMenu>();
	testMenu->RegisterTest("EventTest", []() {return Aether::CreateScope<Test::EventTest>();});
	testMenu->RegisterTest("ShowModel", []() {return Aether::CreateScope<Test::ShowModelTest>();});
	testMenu->RegisterTest("FrameBufferTest", []() {return Aether::CreateScope<Test::FrameBufferTest>();});
	testMenu->RegisterTest("ShaderTest", []() {return Aether::CreateScope<Test::ShaderTest>();});
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
	
}

void Sandbox::OnDestory()
{
	if (m_Test)
		delete m_Test.release();
}

void Sandbox::OnUpdate(float sec)
{
	m_Test->OnUpdate(sec);
}

void Sandbox::OnLoopEnd()
{
	m_Test->OnLoopEnd();
}

void Sandbox::OnLoopBegin()
{
	m_Test->OnLoopBegin();
}
