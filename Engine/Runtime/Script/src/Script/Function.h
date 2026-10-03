#pragma once

#include <functional>
#include <memory>
#include <utility>

#if defined(_WIN32)
#define AETHER_SCRIPT_CALL __stdcall
#else
#define AETHER_SCRIPT_CALL
#endif

namespace Aether::Script
{
class Runtime;

template <typename Signature>
class Function;

// A callable managed function, independent of the backend used to execute it.
// No native address accessor or conversion is exposed.
template <typename Result, typename... Args>
class Function<Result(Args...)>
{
public:
    Function() = default;
    explicit operator bool() const noexcept { return static_cast<bool>(m_Invoke); }

    Result operator()(Args... args) const
    {
        return m_Invoke(std::forward<Args>(args)...);
    }

private:
    friend class Runtime;
    explicit Function(std::function<Result(Args...)> invoke) : m_Invoke(std::move(invoke)) {}

    // Current hostfxr backend adapter. A future interpreter or AOT backend can
    // supply another invocation implementation without changing this API.
    static Function BindNative(void* address, std::shared_ptr<const void> lifetime)
    {
        using EntryPoint = Result (AETHER_SCRIPT_CALL*)(Args...);
        const auto entry = reinterpret_cast<EntryPoint>(address);
        return Function([entry, lifetime = std::move(lifetime)](Args... args) -> Result
        {
            return entry(std::forward<Args>(args)...);
        });
    }

    std::function<Result(Args...)> m_Invoke;
};
}
