#include "TestLayer.h"
#include "Test/TestMenu.h"

#include "Test/ShaderTest.h"
#include "Test/FrameBufferTest.h"
Aether::TestLayer::TestLayer()
{
	auto testMenu = Aether::CreateScope<TestMenu>();
	testMenu->RegisterTest("ShaderTest", []() {return Aether::CreateScope<ShaderTest>();});
	testMenu->RegisterTest("FrameBufferTest", []() {return Aether::CreateScope<FrameBufferTest>();});
	m_Test = std::move(testMenu);
}
void Aether::TestLayer::OnImGuiRender()
{
	m_Test->OnImGuiRender();
	ImGui::Begin("Sandbox");
	ImGui::Text("Hello");
	ImGui::End();
	
}

void Aether::TestLayer::OnRender()
{
	m_Test->OnRender();
}

void Aether::TestLayer::OnEvent(Aether::Event& event)
{
	m_Test->OnEvent(event);
}

void Aether::TestLayer::OnUpdate(float sec)
{
	m_Test->OnUpdate(sec);
}

void Aether::TestLayer::OnLoopEnd()
{
	m_Test->OnLoopEnd();
}

void Aether::TestLayer::OnLoopBegin()
{
	m_Test->OnLoopBegin();
}
