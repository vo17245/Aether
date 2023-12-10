#pragma once
#include "Aether/Utils/TarjanAlgorithm.h"
#include "Model.h"
namespace Aether
{
    bool Model::HasCycle()
    {
        auto graph=TarjanAlgorithm(nodes.size());
        for(size_t i=0;i<nodes.size();i++)
        {
            auto& node=nodes[i];
            for(const auto& child:node.children)
            {
                graph.AddEdge(i, child);
            }
        }
        auto sccs=graph.Tarjan();
        return sccs.size()!=0;
    }
}