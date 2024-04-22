#pragma once
#include <memory>

namespace Aether
{
    template<typename T,typename Deleter=std::default_delete<T>>
    class LocalRef
    {
    public:
        LocalRef(T* ptr,Deleter deleter=Deleter())
            :m_Ptr(ptr),m_Deleter(deleter)
        {
            m_RefCount=new size_t();
            *m_RefCount=1;
        }
        LocalRef(const LocalRef<T,Deleter>& other)
            :m_Ptr(other.m_Ptr),m_RefCount(other.m_RefCount),
            m_Deleter(other.m_Deleter)
        {
            if(m_Ptr!=nullptr)
                (*m_RefCount)++;
        }
        LocalRef(LocalRef<T,Deleter>&& other)
            :m_Ptr(other.m_Ptr),m_RefCount(other.m_RefCount),
            m_Deleter(other.m_Deleter)
        {
            other.m_Ptr=nullptr;
            other.m_RefCount=nullptr;
        }
        LocalRef<T,Deleter>& operator=(const LocalRef<T,Deleter>& other)
        {
            if(this==&other)
                return;
            m_Ptr=other.m_Ptr;
            m_RefCount=other.m_RefCount;
            m_Deleter=other.m_Deleter;
            if(m_Ptr==nullptr)
                return 
            (*m_RefCount)++;


            return *this;
        }
        LocalRef<T,Deleter>& operator=(LocalRef<T,Deleter>&& other)
        {
            if(this==&other)
                return;
            m_Ptr=other.m_Ptr;
            m_RefCount=other.m_RefCount;
            m_Deleter=other.m_Deleter;
            other.m_Ptr=nullptr;
            other.m_RefCount=nullptr;
            return *this;
        }
        ~LocalRef()
        {
            if(m_Ptr==nullptr)
                return;
            (*m_RefCount)--;
            if(*m_RefCount==0)
            {
                delete m_RefCount;
                m_Deleter(m_Ptr);            
            }
        }
        T* operator->()
        {
            return m_Ptr;
        }
        T& operator*()
        {
            return *m_Ptr;
        }
        size_t GetRefCount()
        {
            return *m_RefCount;
        }
        T* GetRaw()
        {
            return m_Ptr;
        }
    private:
        T* m_Ptr;
        size_t* m_RefCount;
        Deleter m_Deleter;
    };

    template<typename T,typename... args>
    LocalRef<T> CreateLocalRef(args&&... Args)
    {
        return LocalRef<T>(new T(std::forward<args>(Args)...));
    }
}//namespace Aether