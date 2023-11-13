#pragma once
#include "Aether/Core/Layer.h"
#include "Test/Test.h"
namespace Aether
{
	class TestLayer:public Layer
	{
	public:
		TestLayer();
		void OnImGuiRender()override;
		void OnRender()override;
		void OnEvent(Aether::Event& event)override;
		void OnUpdate(float sec)override;
		void OnLoopEnd()override;
		void OnLoopBegin()override;
	private:
		Aether::Scope<Test> m_Test;
	};
}