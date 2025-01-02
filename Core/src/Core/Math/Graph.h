#pragma once
#include <utility>
#include <vector>

namespace Aether {
namespace Math {
/**
 * 
*/
class Graph
{
public:
    Graph(size_t n)
    {
        m_Adj.resize(n);
        for (size_t i = 0; i < n; ++i)
        {
            m_Adj[i].resize(n, 0);
        }
    }
    /**
     * @note 在拓扑排序中表示to依赖from
    */
    void AddEdge(size_t from, size_t to)
    {
        m_Adj[from][to] = 1;
    }
    std::vector<std::vector<size_t>> TopologicalSort()
    {
        // get deps
        std::vector<size_t> deps;
        size_t n = m_Adj.size();
        for (size_t i = 0; i < n; ++i)
        {
            size_t dep = 0;
            for (size_t j = 0; j < n; ++j)
            {
                dep += m_Adj[j][i];
            }
            deps.emplace_back(dep);
        }
        // search node deps=0
        std::vector<size_t> cur;
        for (size_t i = 0; i < n; ++i)
        {
            if (deps[i] == 0)
            {
                cur.emplace_back(i);
            }
        }
        // sort
        std::vector<std::vector<size_t>> timelines;
        std::vector<size_t> next;
        while (!cur.empty())
        {
            timelines.emplace_back(cur);
            for (auto& c : cur)
            {
                for (size_t i = 0; i < n; ++i)
                {
                    if (m_Adj[c][i] == 1)
                    {
                        deps[i]--;
                        if (deps[i] == 0)
                        {
                            next.emplace_back(i);
                        }
                    }
                }
            }
            std::swap(cur, next);
            next.clear();
        }
        return timelines;
    }

private:
    // 有向邻接矩阵
    // m_Adj[i][j] 为1 表示顶点j 指向 顶点i 在拓扑排序中表示j依赖i
    std::vector<std::vector<int>> m_Adj;
};
}
} // namespace Aether::Math