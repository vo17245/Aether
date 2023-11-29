#include "EditorLayer.h"
#include "Aether/Core/Core.h"
#include "Aether/Scene/Component.h"
#include "Aether/Asset/ModelAssetImporter.h"
#include "Aether/Core/Config.h"
#include "Aether/Scene/VisualServer.h"
#include "Aether/Render/Model.h"
#include "Aether/Scene/SceneSerializer.h"
Aether::EditorLayer::EditorLayer()
	: m_CameraController(45,0.01,100,1600.0/900)
{
	auto scene = CreateRef<Scene>();
	AttachScene(scene);
	m_FB = Aether::CreateRef<Aether::FrameBuffer>(1600, 900);
	OpenGLApi::BindFrameBuffer(0);
	auto teapot=m_Scene->CreateEntity();
	std::string teapotModelPath = std::string(GetConfig().resource_path) + "/Model/teapot.obj";
	auto res1=ModelAssetImporter::LoadFromFile(teapotModelPath);
	
	if (!res1)
	{
		Log::Error("failed to load {}", teapotModelPath);
		return;
	}
	auto teapotModel = CreateRef<Model>(res1.value());
	teapot.AddComponent<VisualComponent>(teapotModel);
	teapot.AddComponent<TagComponent>("teapot");
	teapot.AddComponent<TransformComponent>();
	auto light1 = m_Scene->CreateEntity();
	light1.AddComponent<PointLightComponent>(Vec3(1, 1, 1), Vec3(100, 100, 100));
	light1.AddComponent<TagComponent>("light1");
	auto light2 = m_Scene->CreateEntity();
	light2.AddComponent<PointLightComponent>(Vec3(1, 1, 1), Vec3(-100, 100, 100));
	light2.AddComponent<TagComponent>("light2");
	auto cube= m_Scene->CreateEntity();
	std::string cubeModelPath = std::string(GetConfig().resource_path) + "/Model/cube.glb";
	auto res2 = ModelAssetImporter::LoadFromFile(cubeModelPath);
	AETHER_ASSERT(res2);
	auto cubeModel = CreateRef<Model>(res2.value());
	cube.AddComponent<VisualComponent>(cubeModel);
	cube.AddComponent<TagComponent>("cube");
	cube.AddComponent<TransformComponent>();
}

Aether::EditorLayer::~EditorLayer()
{
	
}

void Aether::EditorLayer::OnImGuiRender()
{
	
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	{
		static bool p_open = true;
		static int window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoResize|
			ImGuiWindowFlags_NoMove|
			ImGuiWindowFlags_NoTitleBar;
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::Begin("dockspace", &p_open, window_flags);
		ImGui::End();
	}
	{
		static bool p_open = true;
		static int window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_MenuBar|
			ImGuiWindowFlags_NoTitleBar;
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::Begin("Editor", &p_open, window_flags);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					CameraStorage cameraStorage(m_CameraController.GetCamera());

					SceneSerializer::WriteFile(*m_Scene,"first", 
						cameraStorage, "../../Data/first.json");
				}
				if(ImGui::MenuItem("Load"))
				{
					auto opt_res=SceneSerializer::LoadFile("../../Data/first.json");
					if(opt_res)
					{
						m_EntityTextInputBuffer.clear();
						DeserializationResult& res=opt_res.value();
						AttachScene(res.scene);
						m_CameraController.GetCamera()=res.camera.perspectiveCamera.value();
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("Add entity"))
				{
					m_Scene->CreateEntity();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}
	ImGui::Begin("center");
	ImGui::End();
	DrawSceneHierachyPanel();
	DrawViewerPanel();
	
}

void Aether::EditorLayer::OnRender()
{
	
	m_FB->Bind();
	OpenGLApi::SetViewport(0, 0, m_FB->GetTexture()->GetWidth(), m_FB->GetTexture()->GetHeight());
	OpenGLApi::SetClearColor(0.4, 0.5, 0.6, 1.0);
	OpenGLApi::ClearColorBuffer();
	OpenGLApi::ClearDepthBuffer();
	m_CameraController.GetCamera().CalculateProjection();
	m_CameraController.GetCamera().CalculateView();
	VisualServer::Render(*m_Scene,m_CameraController.GetCamera());
	m_FB->Unbind();
}

void Aether::EditorLayer::OnEvent(Event& e)
{
	
}

void Aether::EditorLayer::OnUpdate(float ds)
{
	
	m_CameraController.OnUpdate(ds);
}

void Aether::EditorLayer::AttachScene(Ref<Scene>& scene)
{
	m_Scene = scene;
	m_Scene->GetOnCreate() = AETHER_BIND_FN(OnEntityCreate);
	m_Scene->GetOnDelete() = AETHER_BIND_FN(OnEntityDelete);
	auto view = m_Scene->GetAllEntitiesWith<IDComponent>();
	for (auto& [entity, ic] : view.each())
	{
		Entity e = { entity, m_Scene.get() };
		OnEntityCreate(e);
	}
	m_SelectedEntity.reset();
}

void Aether::EditorLayer::DrawSceneHierachyPanel()
{
	constexpr const size_t BUF_SIZE = 128;
	static char buf[BUF_SIZE] = {0};
	ImGui::Begin("SceneHierachy");
	auto view = m_Scene->GetAllEntitiesWith<IDComponent>();
	for (const auto& [entity, ic] : view.each())
	{
		Entity e = { entity,m_Scene.get()};
		std::string displayName;
		if(e.HasComponent<TagComponent>())
		{
			auto& tgc=e.GetComponent<TagComponent>();
			displayName=tgc.tag;
		}
		else 
		{
			displayName=fmt::format("[{}]",std::to_string(uint64_t(ic.id)));
		
		}
		ImGui::PushID(ic.id);
		if (ImGui::TreeNode(displayName.c_str()))
		{
			m_SelectedEntity=e;
			if(e.HasComponent<TagComponent>())
			{
				auto& entityTextInputBuf = m_EntityTextInputBuffer[ic.id];
				auto& tgc = e.GetComponent<TagComponent>();
				if (ImGui::TreeNode("Tag"))
				{
					ImGui::Text("value: %s",tgc.tag.c_str());
					ImGui::Text("input buffer: %s", (const char*)entityTextInputBuf.data());
					//ImGui::Text("edit buffer: %s", entity_buf.c_str());
					ImGui::InputText("tag", (char*)entityTextInputBuf.data(),
						entityTextInputBuf.size());
					if (ImGui::Button("apply edit"))
					{
						tgc.tag = (const char*)entityTextInputBuf.data();
					}
					ImGui::TreePop();
				}
			}
			if (e.HasComponent<TransformComponent>())
			{
				if (ImGui::TreeNode("Transform"))
				{
					auto& tc = e.GetComponent<TransformComponent>();
					ImGui::InputFloat3("position", tc.position.data());
					ImGui::TreePop();
				}
			}
			if (e.HasComponent<PointLightComponent>())
			{
				auto& lc = e.GetComponent<PointLightComponent>();

			}
			if (e.HasComponent<PointLightComponent>())
			{
				if (ImGui::TreeNode("PointLight"))
				{
					auto& lc = e.GetComponent<PointLightComponent>();
					ImGui::InputFloat3("color", lc.light.GetColor().data());
					ImGui::InputFloat3("position", lc.light.GetPosition().data());
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();

	}
	if (m_SelectedEntity)
	{
		auto& e = m_SelectedEntity.value();
		std::string displayName;
		if (e.HasComponent<TagComponent>())
		{
			displayName = e.GetComponent<TagComponent>().tag;
		}
		else
		{
			displayName = std::to_string(e.GetComponent<IDComponent>().id);
		}
		ImGui::Text("selected: %s", displayName.c_str());
		if (ImGui::Button("add component"))
		{

		}
		if (ImGui::Button("remove component"))
		{

		}
	}
	ImGui::End();
}

bool Aether::EditorLayer::OnWindowResize(WindowResizeEvent& e)
{
	return false;
}

void Aether::EditorLayer::DrawViewerPanel()
{
	static int window_flags = ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar;
	static bool open = true;
	ImGui::Begin("SceneView", &open, window_flags);
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::Image((void*)m_FB->GetTexture()->GetRendererId(),
		ImVec2(m_FB->GetTexture()->GetWidth(),
			m_FB->GetTexture()->GetHeight()));
	ImGui::End();

}

namespace Aether
{
	void EditorLayer::OnEntityCreate(Entity& entity)
	{
		auto& ic=entity.GetComponent<IDComponent>();
		
		m_EntityTextInputBuffer[ic.id]=std::move(std::vector<std::byte>(128));
	}
	void EditorLayer::OnEntityDelete(Entity& entity)
	{
		auto& ic=entity.GetComponent<IDComponent>();
		auto iter= m_EntityTextInputBuffer.find(ic.id);
		m_EntityTextInputBuffer.erase(iter);
	}
}