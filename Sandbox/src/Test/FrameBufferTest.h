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
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Geometry/Rectangle.h"
namespace Aether
{
	class FrameBufferTest :public Test
	{

	public:
		FrameBufferTest();
		~FrameBufferTest();
		void OnRender()override;
		void OnImGuiRender()override;
	private:
		Aether::Ref<Aether::Mesh> m_Rectangle;
		Aether::Ref<Aether::FrameBuffer> m_FB;
		Aether::OrthographicCamera m_OrthographicCamera;
		Aether::Ref<Aether::Shader> m_ScreenShader;
		Aether::Scene m_Scene;
		Aether::Ref<Aether::Shader> m_Shader;
		Aether::PerspectiveCamera m_Camera;
		Aether::Ref<Aether::Texture2D> m_Texture;
	};
}