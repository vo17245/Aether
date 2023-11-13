#pragma once

#include "Test.h"
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include "Aether/Core/Log.h"
namespace Aether
{
	class TestMenu :public Test
	{
	public:
		TestMenu() {};
		~TestMenu() {  };
		void OnImGuiRender()override;
		void OnRender()override;
		void RegisterTest(const std::string& name, std::function<std::unique_ptr<Test>()> creator);
		void OnEvent(Aether::Event& event)override;
		void OnUpdate(float sec)override;
		void OnLoopEnd()override;
		void OnLoopBegin()override;
	private:
		std::unordered_map<std::string, std::function<std::unique_ptr<Test>()>> m_Tests;
		std::unique_ptr<Test> m_CurTest;
	};
}
