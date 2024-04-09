#pragma once
#include <vector>
template<typename T,typename E>
class Result
{
public:
    Result(const T& v)
        :m_Value(v),m_IsOK(true){}
    Result (const E& e)
        :m_Error(e),m_IsOK(false){}
    Result(T&& v)
        :m_Value(std::move(v)),m_IsOK(true){}
    Result (E&& e)
        :m_Error(std::move(e)),m_IsOK(false){}
    Result(Result&& r)
    {
        m_IsOK=r.m_IsOK;
        if(m_IsOK)
            new(&m_Value) T(std::move(r.m_Value));
        else
            new(&m_Error) E(std::move(r.m_Error));
    }
    Result(const Result& r)
    {
        m_IsOK=r.m_IsOK;
        if(m_IsOK)
            new(&m_Value) T(r.m_Value);
        else
            new(&m_Error) E(r.m_Error);
    }
    ~Result()
    {
        if(m_IsOK)
            m_Value.~T();
        else
            m_Error.~E();
    }
    bool IsOK() const
    {
        return m_IsOK;
    }
    const T& GetValue() const
    {
        return m_Value;
    }
    const E& GetError()const
    {
        return m_Error;
    }
    T& GetValue() 
    {
        return m_Value;
    }
    E& GetError()
    {
        return m_Error;
    }
    static Result OK(const T& r)
    {
        Result res(r);
        return res;
    }    
    static Result Err(const E& e)
    {
        Result res(e);
        return res;
    }
private:
    bool m_IsOK;
    union{
        T m_Value;
        E m_Error;
    };
    
};
