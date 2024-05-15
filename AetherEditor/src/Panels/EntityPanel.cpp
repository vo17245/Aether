#include "EntityPanel.h"
#include "Aether/ImGui/imgui.h"
#include "Aether/Resource/Model/ModelLoader.h"
#include "Aether/Scene/Component.h"
#include "Aether/Scene/Reflection.h"
#include "Core/MainScene.h"
#include <stdint.h>
#include "Aether/Scene.h"
#include "Panels/EntityPanel.h"
#include "UI/ImGuiStorage.h"
#include "UI/UICommandHandler.h"
#include "Aether/Utils/FileUtils.h"
namespace Aether {
namespace Editor {
using AllComponent = Aether::Reflection::ComponentTypeGroup<
    TagComponent,
    IDComponent,
    TransformComponent,
    SkyboxComponent,
    MeshComponent,
    PbrMeterialComponent,
    LuaCameraScriptComponent,
    LuaScriptComponent>;
static std::string GetBriefName(const std::string& fullName)
{
    std::string res;
    // 去掉命名空间
    auto pos = fullName.find_last_of("::");
    if (pos == std::string::npos)
    {
        return fullName;
    }
    res = fullName.substr(pos + 1);
    ////去掉component 后缀
    // pos = res.find("Component");
    // if (pos == std::string::npos)
    //{
    //     return res;
    // }
    res = res.substr(0, pos);
    return res;
}
template <typename ComponentT>
struct FieldView
{
    const char* name;
    ComponentT& component;
    FieldView(const char* _name, ComponentT& _component) :
        name(_name), component(_component)
    {}
    template <typename T>
    void operator()(T& field)
    {
        ImGui::Text(name);
        // AETHER_DEBUG_LOG_ERROR("not implemented field type imgui edit ui {}", typeid(T).name());
    }
    template <>
    void operator()(std::string& field)
    {
        // 获得当前id
        ImGuiID id = ImGui::GetID(name);
        // string 存储位置
        char* storage = ImGuiStorage::Get(id);
        if (storage[0] == 0)
        {
            // 初始化
            size_t bytes_to_copy = std::min(ImGuiStorage::MAX_STORAGE_SIZE - 1,
                                            field.size());
            memcpy(storage + 1, field.c_str(), bytes_to_copy);
            storage[0] = 1;
        }
        bool ret = ImGui::InputText(name, storage + 1, ImGuiStorage::MAX_STORAGE_SIZE - 1);
        if (ret)
        {
            Reflection::Meta<ComponentT>::Set(component, name, std::string(storage));
        }
    }
    template <>
    void operator()(UUID& uuid)
    {
        ImGui::Text(fmt::format("{}: {}", name, size_t(uuid)).c_str());
    }
    template <>
    void operator()(size_t& val)
    {
        ImGui::Text(fmt::format("{}: {}", name, val).c_str());
    }
    template <>
    void operator()(float& val)
    {
        ImGui::Text(fmt::format("{}: {}", name, val).c_str());
    }
    template <>
    void operator()(Eigen::Vector3f& val)
    {
        ImGui::Text(fmt::format("{}: {} {} {}", name, val.x(), val.y(), val.z()).c_str());
    }
    template <>
    void operator()(std::optional<std::string>& val)
    {
        if (val)
        {
            ImGui::Text(fmt::format("{}: {}", name, val.value()).c_str());
        }
        else
        {
            ImGui::Text(fmt::format("{}: {}", name, "[NULL]").c_str());
        }
    }
};
template <typename ComponentT>
struct FieldHandler
{
    ComponentT& component;
    FieldHandler(ComponentT& _component) :
        component(_component)
    {}
    void operator()(const char* name, const char* type, const char* comment,
                    Reflection::CoreComponentFieldVariant value)
    {
        std::visit(FieldView(name, component), value);
    }
};

struct ComponentHandler
{
    ComponentHandler(Entity& _e, UICommandHandler& commandHandler) :
        e(_e), m_CommandHandler(commandHandler)
    {}
    Entity& e;
    UICommandHandler& m_CommandHandler;
    template <typename T>
    void operator()(Reflection::ComponentType<T>)
    {
        if (e.HasComponent<T>())
        {
            auto& c = e.GetComponent<T>();
            if (ImGui::TreeNode(
                    GetBriefName(
                        std::string(Aether::Reflection::GetComponentTypeString<T>()))
                        .c_str()))
            {
                Aether::Reflection::ForEachField(c, FieldHandler<T>(c));
                bool removeComponent = false;
                if constexpr (!std::is_same_v<IDComponent, T>)
                {
                    removeComponent = ImGui::Button("Remove");
                }
                ImGui::TreePop();
                if constexpr (!std::is_same_v<IDComponent, T>)
                {
                    if (removeComponent)
                    {
                        e.RemoveComponent<T>();
                    }
                }
            }
        }
    }
    template <>
    void operator()(Reflection::ComponentType<MeshComponent>)
    {
        if (e.HasComponent<MeshComponent>())
        {
            if (ImGui::TreeNode(
                    GetBriefName(
                        std::string(Aether::Reflection::GetComponentTypeString<MeshComponent>()))
                        .c_str()))
            {
                auto& c = e.GetComponent<MeshComponent>();
                Aether::Reflection::ForEachField(c, FieldHandler<MeshComponent>(c));
                bool reloadMeshClicked = ImGui::Button("reload mesh");
                if (reloadMeshClicked)
                {
                    m_CommandHandler.PushCommand("reload_selected_entity_mesh");
                }

                ImGui::TreePop();
            }
        }
    }
    template <>
    void operator()(Reflection::ComponentType<LuaCameraScriptComponent>)
    {
        if (e.HasComponent<LuaCameraScriptComponent>())
        {
            if (ImGui::TreeNode(
                    GetBriefName(
                        std::string(Aether::Reflection::GetComponentTypeString<LuaCameraScriptComponent>()))
                        .c_str()))
            {
                auto& c = e.GetComponent<LuaCameraScriptComponent>();
                Aether::Reflection::ForEachField(c, FieldHandler<LuaCameraScriptComponent>(c));
                bool reloadMeshClicked = ImGui::Button("reload script");
                if (reloadMeshClicked)
                {
                    m_CommandHandler.PushCommand("reload_selected_entity_lua_camera_script");
                }

                ImGui::TreePop();
            }
        }
    }
};
void EntityPanel::OnImGuiRender()
{
    auto& mainScene = MainScene::GetInstance();
    bool removeEntity = false;
    ImGui::Begin("Entity Panel");
    if (mainScene.HasEntitySelected())
    {
        auto& entity = mainScene.GetEntitySelected();
        auto& ic = entity.GetComponent<IDComponent>();
        ImGui::PushID(ic.id);
        auto handler = ComponentHandler(entity, m_CommandHandler);
        Reflection::ForEachComponentType(AllComponent{}, handler);

        if (ImGui::Button("Add Component"))
        {
            auto* msg = new Message::SelectComponentBegin();
            MessageBus::GetSingleton()
                .Publish<Message::SelectComponentBegin>(nullptr);
        }
        removeEntity = ImGui::Button("Remove Entity");
        ImGui::PopID();
    }
    else
    {
        ImGui::Text("no entity selected");
    }

    ImGui::End();
    if (removeEntity)
    {
        // 获取选中的entity
        auto& entity = MainScene::GetInstance().GetEntitySelected();
        // 从MainScene 移除选中
        MainScene::GetInstance().CancelSelect();
        // 删除entity
        MainScene::GetInstance().GetScene().DeleteEntity(entity);
    }
}
void EntityPanel::OnUpdate(Real sec)
{
    bool res = m_CommandHandler.DoAllCommand();
    if (!res)
    {
        AETHER_DEBUG_LOG_ERROR("do command failed");
    }
}
void EntityPanel::RegisterUICommand()
{
    // reload_selected_entity_mesh
    m_CommandHandler.RegisterCommand("reload_selected_entity_mesh", []() {
        if (!MainScene::GetInstance().HasEntitySelected())
        {
            return;
        }
        Entity& entity = MainScene::GetInstance().GetEntitySelected();
        if (!entity.HasComponent<MeshComponent>())
        {
            return;
        }
        auto& mc = entity.GetComponent<MeshComponent>();
        if (!mc.filePath)
        {
            return;
        }
        auto& filePath = mc.filePath.value();
        auto model = ModelLoader::LoadFromFile(filePath);
        if (!model)
        {
            return;
        }
        mc.model = model;
    });
    // reload_selected_entity_lua_camera_script
    m_CommandHandler.RegisterCommand("reload_selected_entity_lua_camera_script", []() {
        if (!MainScene::GetInstance().HasEntitySelected())
        {
            return;
        }
        Entity& entity = MainScene::GetInstance().GetEntitySelected();
        if (!entity.HasComponent<LuaCameraScriptComponent>())
        {
            return;
        }
        auto& lsc = entity.GetComponent<LuaCameraScriptComponent>();

        auto& filePath = lsc.path;
        auto script = FileUtils::ReadFileAsString(filePath);
        if (!script)
        {
            return;
        }
        lsc.script = std::move(script.value());
    });
}
}
} // namespace Aether::Editor