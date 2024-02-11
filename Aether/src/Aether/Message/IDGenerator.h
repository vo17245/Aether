#pragma once
#include <vector>
namespace Aether
{
    class IDGenerator
    {
    public:
        IDGenerator() = default;
        ~IDGenerator()= default;
        size_t Get()
        {
            if(m_IdPool.empty())
            {
                return m_Next++;
            }
            else
            {
                size_t id = m_IdPool.back();
                m_IdPool.pop_back();
                return id;
            }
        }
        size_t Release(size_t id)
        {
            m_IdPool.emplace_back(id);
        }
    private:
        size_t m_Next;
        std::vector<size_t> m_IdPool;
    };

}