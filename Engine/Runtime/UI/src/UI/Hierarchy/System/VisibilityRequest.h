#pragma once
#include "System.h"
#include "../Component/VisibilityRequest.h"
#include "../Node.h"
#include "../Component/Text.h"
#include "../Component/Quad.h"
#include "../Component/Node.h"
namespace Aether::UI
{
class VisibilityRequestSystem : public SystemI
{
public:
    virtual void OnUpdate(float sec, World& scene)
    {
        auto view=scene.Select<VisibilityRequestComponent>();
        for(const auto& [entity,vrc]:view.each())
        {
            if(vrc.processed==false)
            {
                ProcessVisibilityRequest(entity, vrc.visible,scene);
            }
        }
    }
private:
    void ProcessVisibilityRequest(entt::entity entity, bool visible,World& scene)
    {
        if(scene.HasComponent<TextComponent>(entity))
        {
            auto& text= scene.GetComponent<TextComponent>(entity);
            text.visible = visible;
        }
        if(scene.HasComponent<QuadComponent>(entity))
        {
            auto& quad= scene.GetComponent<QuadComponent>(entity);
            quad.visible = visible;
        }
        if(scene.HasComponent<NodeComponent>(entity))
        {
            auto& nodeComponent = scene.GetComponent<NodeComponent>(entity);
            for (auto child : nodeComponent.node->children)
            {
                ProcessVisibilityRequest(child->id, visible, scene);
            }
        }
    }
};
} // namespace Aether::UI