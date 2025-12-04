#pragma once
#include <utility>
#include <vector>
#include <functional>
#include <optional>
namespace Aether::UI
{

template <typename T,typename Creator>
class Pool
{
public:
    class Resource
    {
    public:
        Resource(T&& resource, Pool* pool) :
            m_Resource(std::forward<T>(resource)), m_Pool(pool)
        {
        }
        Resource()=default;
        Resource(Resource&&) = default;
        Resource& operator=(Resource&&) = default;
        ~Resource()
        {
            if(m_Resource && m_Pool)
            {
                m_Pool->Return(std::move(*m_Resource));
            }
        }
        T& Get()
        {
            return *m_Resource;
        }
        operator bool()
        {
            return m_Resource.has_value();
        }
        /**
         * @note after call Unwrap, the resource will be invalid
         *       T will not be returned to pool
         * 
        */
        T Unwrap()
        {
            auto t=std::move(m_Resource.value());
            m_Resource.reset();
            return t;
        }
    private:
        std::optional<T> m_Resource;
        Pool* m_Pool=nullptr;
        
    };
    Pool() = default;
    Pool(const Pool&) = delete;
    Pool(Pool&&) = default;
    Pool& operator=(const Pool&) = delete;
    Pool& operator=(Pool&&) = default;
    Resource GetOrCreate()
    {
        if (m_Pool.empty())
        {
            return Resource(m_Create(), this);
        }
        T t = std::move(m_Pool.back());
        m_Pool.pop_back();
        return Resource(std::move(t),this);
    }
    void Return(T&& resource)
    {
        m_Pool.push_back(std::move(resource));
    }
    void SetCreator(Creator&& create)
    {
        m_Create=std::forward<Creator>(create);
    }

private:
    std::vector<T> m_Pool;
    Creator m_Create;
};
} // namespace Aether::UI