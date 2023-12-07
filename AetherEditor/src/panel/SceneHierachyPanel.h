#pragma once
#include "Aether/Scene/Scene.h"
namespace Aether
{
class SceneHierachyPanel
{
public:
    SceneHierachyPanel(){}
    SceneHierachyPanel(const SceneHierachyPanel&)=delete;
    SceneHierachyPanel(SceneHierachyPanel&&)=delete;
    ~SceneHierachyPanel(){}
    void AttachScene(Ref<Scene>& scene);
    void Open(){m_Open=true;};
    void Close(){m_Open=false;};
    bool IsOpen(){return m_Open;}
    void Show(){m_Show=true;}
    void Hide(){m_Show=false;}
    bool IsShow()const{return m_Show;}
    void OnImGuiRender();
    std::optional<Entity> SelectedEntity()const{return m_SelectedEntity;}
    void OnEntityCreate(Entity& entity);
    void OnEntityDestory(Entity& entity);
private:
    Ref<Scene> m_Scene;
    bool m_Open;
    bool m_Show;
    std::optional<Entity> m_SelectedEntity;
    std::unordered_map<UUID,std::vector<std::byte>> m_EntityTextInputBuffer;
    
};
}