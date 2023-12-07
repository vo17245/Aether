#pragma once
#include "Aether/Core/Layer.h"
#include "Aether/Scene/Scene.h"
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Core/Application.h"
#include "Aether/Event/WindowEvent.h"
#include "Aether/Core/Math.h"
#include "Aether/Scene/CameraController.h"
#include "panel/SceneHierachyPanel.h"
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
		Ref<Scene> m_Scene;
		Ref<FrameBuffer> m_FB;
		void AttachScene(Ref<Scene>& scene);
	private:
		
		bool OnWindowResize(WindowResizeEvent& e);
		void DrawViewerPanel();
		std::unordered_map<UUID,std::vector<std::byte>> m_EntityTextInputBuffer;
		void OnEntityCreate(Entity& entity);
		void OnEntityDestory(Entity& entity);
	private:
		SceneHierachyPanel m_SceneHierachyPanel;
		std::vector<std::function<void(Entity&)>> m_EntityCreateHandlers;
		std::vector<std::function<void(Entity&)>> m_EntityDestoryHandlers;

		
	};

}