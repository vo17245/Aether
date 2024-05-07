#pragma once
#include <cstdlib>
#include <vector>
#include "Aether/Core/Log.h"
#include "Log.h"
namespace Aether {

template <typename T>
class Unexpected
{
public:
    T& m_Value;
};
template <typename T, typename E>
class Expected
{
public:
    Expected(const T& value) :
        m_IsError(false), m_Value(value)
    {}
    Expected(const E& error) :
        m_IsError(true), m_Error(error)
    {}
    Expected(const Unexpected<E>& unexpected) :
        m_IsError(true), m_Error(unexpected.m_Value)
    {}
    Expected(T&& value) :
        m_IsError(false), m_Value(std::move(value))
    {}
    Expected(E&& error) :
        m_IsError(true), m_Error(std::move(error))
    {}
    Expected(Unexpected<E>&& unexpected) :
        m_IsError(true), m_Error(std::move(unexpected.m_Value))
    {}
    ~Expected()
    {
        if (m_IsError)
        {
            m_Error.~E();
        }
        else
        {
            m_Value.~T();
        }
    }
    bool IsError() const
    {
        return m_IsError;
    }
    T& Value()
    {
        return m_Value;
    }
    const T& Value() const
    {
        return m_Value;
    }
    E& Error()
    {
        return m_Error;
    }
    const E& Error() const
    {
        return m_Error;
    }
    operator bool() const
    {
        return !m_IsError;
    }
    T& operator*()
    {
        if (m_IsError)
        {
            AETHER_DEBUG_LOG_ERROR("Expected has error");
            std::abort();
        }
        return m_Value;
    }

private:
    bool m_IsError;
    union
    {
        T m_Value;
        E m_Error;
    };
};
} // namespace Aether
