#pragma once 
#include "Aether/Core/Core.h"
#include "Test.h"
#include <string>
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Event/WindowEvent.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Scene/Entity.h"
#include "Aether/Scene/Component.h"
#include "Aether/Render/Camera.h"
namespace Aether
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
		void OnUpdate(float ds)override;
	private:
		Aether::Scene m_Scene;
		Aether::Ref<Aether::Shader> m_Shader;
		Aether::PerspectiveCamera m_Camera;
	};
}