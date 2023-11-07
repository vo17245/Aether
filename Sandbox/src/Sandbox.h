#pragma once
#include "Aether/Core/Application.h"
#include "Test/Test.h"
#include "Test/TestMenu.h"

class Sandbox :public Aether::Application
{
public:
	Sandbox();
	~Sandbox() {}
	void OnImGuiRender()override;
	void OnRender()override;
	void OnEvent(Aether::Event& event)override;
	void OnDestory()override;
	void OnUpdate(float sec)override;
	void OnLoopEnd()override;
	void OnLoopBegin()override;
private:
	Aether::Scope<Test::Test> m_Test;
	
};