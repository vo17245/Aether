#pragma once
#include <vector>
#include <unordered_map>
#include "Aether/Core/Core.h"
namespace Aether
{

class TarjanAlgorithm
{
public:
    TarjanAlgorithm(size_t numVertices)
        :m_NumVertices(numVertices),
        m_Ids(numVertices,-1),
        m_LowLinkValues(numVertices,-1),
        m_OnStack(numVertices,false),
        m_CurrentId(0),
        m_Visited(numVertices,false)

    {
        
    }
    // add a edge: u->v
    void AddEdge(size_t u,size_t v)
    {
        m_Graph[u].emplace_back(v);
    }
    void DFS(size_t vertex)
    {
        m_Visited[vertex]=true;
        m_LowLinkValues[vertex]=m_CurrentId;
        m_Ids[vertex]=m_LowLinkValues[vertex];
        m_CurrentId+=1;
        m_Stack.emplace_back(vertex);
        m_OnStack[vertex]=true;
        for(auto neighbor:m_Graph[vertex])
        {
            if(!m_Visited[neighbor])
            {
                DFS(neighbor);
            }
            if(m_OnStack[neighbor])
            {
                m_LowLinkValues[vertex]=std::min(m_LowLinkValues[vertex],m_LowLinkValues[neighbor]);
            }
        }
        if(m_Ids[vertex]==m_LowLinkValues[vertex])
        {
            std::vector<size_t> component;
            while(true)
            {
                size_t neighbor=m_Stack.back();
                m_Stack.pop_back();
                m_OnStack[neighbor]=false;
                component.emplace_back(neighbor);
                if(neighbor==vertex)
                {
                    break;
                }

            }
            m_StronglyConnectedComponents.emplace_back(std::move(component));
        }

    }
    std::vector<std::vector<size_t>> Tarjan()
    {
        for(size_t i=0;i<m_NumVertices;i++)
        {
            if(m_Ids[i]==-1)
            {
                DFS(i);
            }
        }
        return m_StronglyConnectedComponents;
    }
   
private:
    size_t m_NumVertices;
    std::unordered_map<size_t,std::vector<size_t>> m_Graph;
    std::vector<ssize_t> m_Ids;
    std::vector<ssize_t> m_LowLinkValues;
    std::vector<bool> m_OnStack;
    size_t m_CurrentId;
    std::vector<std::vector<size_t>> m_StronglyConnectedComponents;
    std::vector<size_t> m_Stack;   
    std::vector<bool> m_Visited;
};
}
