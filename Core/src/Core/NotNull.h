#pragma once
#include <cassert>
#include <cstddef>
namespace Aether
{
template <typename T>
class NotNull
{
public:
    NotNull(T* ptr) :
        m_Ptr(ptr)
    {
        assert(ptr != nullptr && "NotNull pointer cannot be null");
    }

    NotNull(const NotNull&) = default;
    NotNull& operator=(const NotNull&) = default;
    NotNull(NotNull&&) noexcept = default;
    NotNull& operator=(NotNull&&) noexcept = default;
    NotNull(std::nullptr_t) = delete;
    NotNull& operator=(std::nullptr_t) = delete;
    NotNull(T& v) :
        m_Ptr(&v)
    {
    }
    T* Get() const
    {
        return m_Ptr;
    }

    T& operator*() const
    {
        return *m_Ptr;
    }

    T* operator->() const
    {
        return m_Ptr;
    }
    operator T*() const
    {
        return m_Ptr;
    }
    operator T&() const
    {
        return *m_Ptr;
    }

private:
    T* m_Ptr;
};
} // namespace Aether