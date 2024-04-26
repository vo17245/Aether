#pragma once
#include "Test.h"
#include "Aether/Scene.h"
namespace Aether
{
	namespace Test
	{
		class ReflectionTest :public Test
		{
		public:
			ReflectionTest() = default;
			~ReflectionTest() = default;
			void OnImGuiRender()override;
		private:
			Scene m_Scene;
			void ShowScene();
		};
	}
	
}