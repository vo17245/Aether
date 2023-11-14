#pragma once
#include "Aether/Core/Layer.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Core/Application.h"
#include "Aether/Event/WindowEvent.h"
#include "Aether/Core/Math.h"
namespace Aether
{
	class EditorLayer:public Layer
	{
	public:
		EditorLayer();
		void OnImGuiRender()override;
		void OnRender()override;
		void OnEvent(Event& e)override;
	
	private:
		Scene m_Scene;
		Ref<FrameBuffer> m_FB;
		Vec2 m_FbViewportSize;
	private:
	
		bool OnWindowResize(WindowResizeEvent& e);

	
	};

}