#pragma once
#include <vector>
namespace Aether
{
    class IDGenerator
    {
    public:
        using ID = uint64_t;
    public:
        IDGenerator() = default;
        ~IDGenerator()= default;
        ID Get()
        {
            if(m_IdPool.empty())
            {
                return m_Next++;
            }
            else
            {
                ID id = m_IdPool.back();
                m_IdPool.pop_back();
                return id;
            }
        }
        void Release(ID id)
        {
            m_IdPool.emplace_back(id);
        }
    private:
        ID m_Next=0;
        std::vector<ID> m_IdPool;
    };

}