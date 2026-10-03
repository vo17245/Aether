#pragma once

#include "Function.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Aether::Script
{
// Hosts framework-dependent IL assemblies in CoreCLR's default load context.
// CoreCLR and its hosting library remain loaded for the process lifetime.
class Runtime
{
public:
    // Uses embedded configuration. Temporary config creation/write/cleanup
    // failures print to stderr and abort; hosting failures populate error.
    static std::optional<Runtime> Create(std::string* error = nullptr);
    static std::filesystem::path GetDotnetRoot();

    bool LoadAssembly(const std::filesystem::path& assembly, std::string* error = nullptr) const;

    // UTF-8 names. typeName is assembly-qualified ("Namespace.Type, Assembly").
    // The current hostfxr backend requires static [UnmanagedCallersOnly]
    // methods. Signature uses ABI-compatible types matching the managed method.
    // Failure returns an empty callable; invoking one throws bad_function_call.
    template <typename Signature>
        requires std::is_function_v<Signature>
    Function<Signature> LoadFunction(std::string_view typeName, std::string_view methodName,
                                     std::string* error = nullptr) const
    {
        const auto address = ResolveFunction(typeName, methodName, error);
        if (!address) return {};
        return Function<Signature>::BindNative(address, m_State);
    }

    // Calls a managed registration method: [UnmanagedCallersOnly] static
    // int Method(IntPtr callback). Zero indicates success. The native callback
    // must remain valid while managed code can call it.
    bool RegisterNativeFunction(std::string_view typeName, std::string_view registrationMethod,
                                void* function, std::string* error = nullptr) const;

    template <typename Function>
        requires (std::is_pointer_v<Function> &&
                  std::is_function_v<std::remove_pointer_t<Function>>)
    bool RegisterNativeFunction(std::string_view typeName, std::string_view registrationMethod,
                                Function function, std::string* error = nullptr) const
    {
        return RegisterNativeFunction(typeName, registrationMethod,
                                      reinterpret_cast<void*>(function), error);
    }

private:
    void* ResolveFunction(std::string_view typeName, std::string_view methodName,
                          std::string* error) const;
    struct State;
    explicit Runtime(std::shared_ptr<const State> state) : m_State(std::move(state)) {}
    std::shared_ptr<const State> m_State;
};
}
