#pragma once 
#include "Aether/Core/Core.h"
#include "Test.h"
#include <string>
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Event/WindowEvent.h"
namespace Test
{
	class ShaderTest :public Test
	{

	public:
		ShaderTest();
		~ShaderTest();
		void OnRender() override;
		void OnImGuiRender() override;
		void OnEvent(Aether::Event& event) override;
		bool OnFileDrop(Aether::WindowFileDropEvent& event);
		void OnLoopEnd()override;
	
	};
}