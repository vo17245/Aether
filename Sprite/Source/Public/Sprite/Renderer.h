#pragma once
#include <Render/RenderApi.h>
#include "Quad.h"
#include <unordered_map>
namespace Aether::Sprite
{
class Renderer
{
public:
    void Begin()
    {
        m_Layers.clear();
        m_ZOrderToLayerIndex.clear();
    }
    void DrawQuad(const Quad& quad)
    {
        // search layer(quads with same zOrder are in the same layer)
        if (quad.zOrder >= m_ZOrderToLayerIndex.size())
        {
            m_ZOrderToLayerIndex.resize(quad.zOrder + 1, UINT32_MAX);
        }
        if (m_ZOrderToLayerIndex[quad.zOrder] == UINT32_MAX)
        {
            m_ZOrderToLayerIndex[quad.zOrder] = (uint32_t)m_Layers.size();
            m_Layers.emplace_back();
        }
        auto& layer = m_Layers[m_ZOrderToLayerIndex[quad.zOrder]];
        // search instance(quads with same atlas are in the same instance)

        for (auto& instance : layer.instances)
        {
            if (instance.atlas == quad.atlas->texture)
            {
                instance.quads.push_back(QuadInstance{quad.affine, quad.CalculateUvOffsetAndScale()});
                return;
            }
        }
        // not found, create new instance
        layer.instances.push_back(InstanceDraw{quad.atlas->texture, {QuadInstance{quad.affine, quad.CalculateUvOffsetAndScale()}}});
    }
    void End(DeviceCommandBuffer& commandBuffer)
    {
        for (auto& index : m_ZOrderToLayerIndex)
        {
            if (index == UINT32_MAX)
            {
                continue;
            }
            auto& layer = m_Layers[index];
            for (auto& instance : layer.instances)
            {
                if (instance.quads.empty())
                {
                    continue;
                }
                DrawInstance(instance);
            }
        }
    }

private:
    struct InstanceDraw
    {
        Borrow<DeviceTexture> atlas;
        std::vector<QuadInstance> quads;
    };
    struct RenderLayer
    {
        std::vector<InstanceDraw> instances;
    };
    std::vector<RenderLayer> m_Layers;
    std::vector<uint32_t> m_ZOrderToLayerIndex;
    DevicePipeline m_Pipeline;
    DeviceSampler m_Sampler;

private:
    void DrawInstance(const InstanceDraw& instance);
};
} // namespace Aether::Sprite