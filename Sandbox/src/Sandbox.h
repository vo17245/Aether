#pragma once
#include "Aether/Core/Application.h"
#include "Test/Test.h"
#include "Test/TestMenu.h"
#include "Aether/UI/CameraController.h"
class Sandbox :public Aether::Application
{
public:
	Sandbox();
	~Sandbox() {}
	void OnImGuiRender()override;
	void OnRender()override;
	void OnEvent(Aether::Event& event)override;
	
private:
	Aether::Scope<Test::Test> m_Test;
	Aether::CameraController m_CameraController;
};