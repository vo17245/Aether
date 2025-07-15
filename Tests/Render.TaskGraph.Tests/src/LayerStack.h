#pragma once
#include <Window/Layer.h>
using namespace Aether;
class LayerStack
{
public:
    void PushLayer(Scope<Layer>&& layer)
    {
        m_Layers.push_back(std::move(layer));
    }
    bool PopLayer(Scope<Layer>&& layer)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it == m_Layers.end())
        {
            return false;
        }
        m_Layers.erase(it);
        return true;
    }
    void SetFirstLayer(Scope<Layer>&& layer)
    {
        m_FirstLayer=std::move(layer);
    }
    Scope<Layer>& GetFirstLayer()
    {
        return m_FirstLayer;
    }
    void Clear()
    {
        m_Layers.clear();
        m_FirstLayer.reset();
    }
    void OnRender()
    {
        
    }


private:
    Scope<Layer> m_FirstLayer;
    std::vector<Scope<Layer>> m_Layers;
};