#pragma once
#include "Test.h"
#include <string>
#include <unordered_map>
#include <functional>
#include "Test/TestMenu.h"
namespace Aether
{
	namespace Test
	{
	class TestRegister
	{
	public:
		static TestRegister& GetSingleton()
		{
			static TestRegister test_register;
			return test_register;
		}
		template<typename T>
		void PushTest(const std::string& name)
		{
			m_Tests[name] = []() {return std::make_unique<T>();};
		}
		Scope<TestMenu> CreateTestMenu()
		{
			auto menu = std::make_unique<TestMenu>();
			for (auto& [name, creator] : m_Tests)
			{
				menu->RegisterTest(name, creator);
			}
			return menu;
		}
		~TestRegister() = default;
	private:
		TestRegister() = default;
		std::unordered_map<std::string, std::function<std::unique_ptr<Test>()>> m_Tests;
	};
	template<typename T>
	class AutoRegisterT
	{
	public:
		AutoRegisterT(const std::string& name)
		{
			TestRegister::GetSingleton().PushTest<T>(name);
		}
		~AutoRegisterT() = default;
	};
	#define REGISTER_TEST(type) static AutoRegisterT<type> AUTO_REGISTER_##type(#type);
	}
}