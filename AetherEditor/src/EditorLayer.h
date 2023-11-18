#pragma once
#include "Aether/Core/Layer.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Core/Application.h"
#include "Aether/Event/WindowEvent.h"
#include "Aether/Core/Math.h"
#include "Aether/Scene/CameraController.h"
namespace Aether
{
	class EditorLayer:public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();
		void OnImGuiRender()override;
		void OnRender()override;
		void OnEvent(Event& e)override;
		void OnUpdate(float ds)override;
	private:
		PerspectiveCameraController m_CameraController;
		Scene m_Scene;
		Ref<FrameBuffer> m_FB;
	
	private:
	
		bool OnWindowResize(WindowResizeEvent& e);

	
	};

}