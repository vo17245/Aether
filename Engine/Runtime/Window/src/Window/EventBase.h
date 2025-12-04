#pragma once
#include <cstdint>
#include <format>
#include <variant>
#include <string>
#include "KeyboardCode.h"
namespace Aether {
template <typename T>
class EventBase
{
public:
    const char* GetName() const
    {
        return (static_cast<const T*>(this))->GetNameImpl();
    }
    std::string ToString() const
    {
        return (static_cast<const T*>(this))->ToStringImpl();
    }
    bool IsHandled() const
    {
        return m_Handled;
    }
    void Handle(bool handled = true)
    {
        m_Handled = handled;
    }

private:
    bool m_Handled = false;
};
} // namespace Aether