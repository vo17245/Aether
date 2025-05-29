#pragma once
#include "Node.h"
#include "System/Quad.h"
#include "System/Text.h"
#include "System/Mouse.h"
#include "System/InputText.h"
#include "System/VisibilityRequest.h"
#include "LayoutBuilder.h"
#include <queue>
namespace Aether::UI
{
inline void BuildLayout(Node* root, Scene& scene, const Vec2f& screenSize, float far, LayoutBuilder& builder)
{
    builder.Begin(screenSize);
    struct NodeInfo
    {
        Node* node;
        size_t containerId;
        float containerZ;
    };
    std::queue<NodeInfo> nodes;
    nodes.push({root, 0, far});
    size_t boxId = 0;
    while (!nodes.empty())
    {
        auto nodeInfo = nodes.front();
        nodes.pop();
        auto* node = nodeInfo.node;
        auto containerId = nodeInfo.containerId;
        auto& base = scene.GetComponent<BaseComponent>(node->id);
        if (base.layoutEnabled)
        {
            base.position = builder.PushBox(base.size, containerId);
            base.z = nodeInfo.containerZ - 1;
        }
        else
        {
            builder.PlaceBox(base.size, base.position);
        }

        boxId++;
        for (auto& child : node->children)
        {
            nodes.push({child, boxId, base.z});
        }
    }
    builder.End();
}
inline void BuildLayout(Node* root, Scene& scene, const Vec2f& screenSize, float far)
{
    LayoutBuilder builder;
    BuildLayout(root, scene, screenSize, far, builder);
}
} // namespace Aether::UI