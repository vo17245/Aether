#include "ReflectionTest.h"
#include "TestRegister.h"



namespace Aether
{
	namespace Test
	{
		struct FieldHandler
		{
			void operator()(const char* name, const char* type, const char* comment,
				Reflection::CoreComponentFieldVariant& value)
			{
				AETHER_DEBUG_LOG(R"(
	{} {}
	{}
)", name, type, comment);
			}
		};
		struct ComponentHandler
		{
			ComponentHandler(Entity _e) :e(_e) {}
			Entity& e;
			template<typename T>
			void operator()(Reflection::ComponentType<T>)
			{
				if (e.HasComponent<T>())
				{
					T& c = e.GetComponent < T>();
					AETHER_DEBUG_LOG("{}",Reflection::GetComponentTypeString<T>());
					Reflection::ForEachField<T>(c, FieldHandler{});
				}
			}
		};
		REGISTER_TEST(ReflectionTest);
		void ReflectionTest::OnImGuiRender()
		{
			ImGui::Begin("Reflection Test");
			bool show_scene = ImGui::Button("show scene");
			bool create_tag_entity = 
				ImGui::Button("create tag entity");
			bool create_transform_entity = 
				ImGui::Button("create transform entity");
			ImGui::End();
			if (create_tag_entity)
			{
				auto e = m_Scene.CreateEntity();
				e.AddComponent<TagComponent>("alice");
			}
			if (create_transform_entity)
			{
				auto e = m_Scene.CreateEntity();
				e.AddComponent<TransformComponent>();
			}
			if (show_scene)
			{
				ShowScene();
			}
		}
		void ReflectionTest::ShowScene()
		{
			using AllComponent = 
				Reflection::ComponentTypeGroup<TagComponent, TransformComponent>;
			auto view = m_Scene.GetAllEntitiesWith<IDComponent>();
			for (auto [entity, ic] : view.each())
			{
				auto e = Entity(entity, &m_Scene);
				Reflection::ForEachComponentType(AllComponent{}, ComponentHandler(e));
			}
		}

	}//namespace Test
}//namespace Aether
