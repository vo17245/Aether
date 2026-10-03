#include "Runtime.h"
#include "ScriptConfig.h"
#include "Private/TemporaryRuntimeConfig.h"
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <cstdint>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace Aether::Script
{
namespace
{
void ClearError(std::string* error)
{
    if (error) error->clear();
}

bool Fail(std::string* error, std::string message)
{
    if (error) *error = std::move(message);
    return false;
}

bool FailStatus(std::string* error, std::string_view operation, int32_t status)
{
    std::ostringstream message;
    message << operation << " (status: 0x" << std::hex << std::setw(8)
            << std::setfill('0') << static_cast<uint32_t>(status) << ')';
    return Fail(error, message.str());
}

std::basic_string<char_t> HostString(std::string_view utf8)
{
#if defined(_WIN32)
    return std::filesystem::path(std::u8string(utf8.begin(), utf8.end())).native();
#else
    return std::string(utf8);
#endif
}

struct HostingApi
{
    hostfxr_initialize_for_runtime_config_fn initialize = nullptr;
    hostfxr_get_runtime_delegate_fn getDelegate = nullptr;
    hostfxr_close_fn close = nullptr;
    std::string error;

    HostingApi()
    {
#if defined(_WIN32)
        const auto library = LoadLibraryW(HostString(AETHER_SCRIPT_HOSTFXR_PATH).c_str());
#else
        const auto library = dlopen(AETHER_SCRIPT_HOSTFXR_PATH, RTLD_LAZY | RTLD_LOCAL);
#endif
        if (!library)
        {
#if defined(_WIN32)
            error = "Failed to load hostfxr (Windows error " + std::to_string(GetLastError()) + ')';
#else
            error = std::string("Failed to load hostfxr: ") + dlerror();
#endif
            return;
        }
        // Do not unload hostfxr: CoreCLR can use it after host contexts close.
        const auto symbol = [library](const char* name)
        {
#if defined(_WIN32)
            return GetProcAddress(library, name);
#else
            return dlsym(library, name);
#endif
        };
        initialize = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(
            symbol("hostfxr_initialize_for_runtime_config"));
        getDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(
            symbol("hostfxr_get_runtime_delegate"));
        close = reinterpret_cast<hostfxr_close_fn>(symbol("hostfxr_close"));
        if (!initialize || !getDelegate || !close)
            error = "hostfxr is missing required hosting symbols";
    }
};

struct HostContext
{
    hostfxr_handle handle = nullptr;
    hostfxr_close_fn close;
    ~HostContext() { if (handle) close(handle); }
};
}

struct Runtime::State
{
    load_assembly_fn loadAssembly = nullptr;
    get_function_pointer_fn getFunction = nullptr;
};

std::filesystem::path Runtime::GetDotnetRoot()
{
    constexpr std::string_view root = AETHER_SCRIPT_DOTNET_ROOT;
    return std::filesystem::path(std::u8string(root.begin(), root.end()));
}

std::optional<Runtime> Runtime::Create(std::string* error)
{
    ClearError(error);
    static const HostingApi api;
    if (!api.error.empty())
    {
        Fail(error, api.error);
        return std::nullopt;
    }

    const auto root = GetDotnetRoot().native();
    // Declared before HostContext so cleanup runs after the context closes,
    // including initialization/delegate failures and exception unwinding.
    const Detail::TemporaryRuntimeConfig runtimeConfig(Detail::RuntimeConfig);
    const auto config = runtimeConfig.Path().native();
    const hostfxr_initialize_parameters parameters{sizeof(hostfxr_initialize_parameters), nullptr, root.c_str()};
    HostContext context{nullptr, api.close};
    const auto result = api.initialize(config.c_str(), &parameters, &context.handle);
    // Positive results also include Success_HostAlreadyInitialized and
    // Success_DifferentRuntimeProperties. Delegates outlive this context.
    if (result < 0 || !context.handle)
    {
        FailStatus(error, "Failed to initialize .NET runtime", result);
        return std::nullopt;
    }

    void* loader = nullptr;
    const auto loaderResult = api.getDelegate(context.handle, hdt_load_assembly, &loader);
    if (loaderResult < 0 || !loader)
    {
        FailStatus(error, "Failed to get IL assembly loader", loaderResult);
        return std::nullopt;
    }
    void* resolver = nullptr;
    const auto resolverResult = api.getDelegate(context.handle, hdt_get_function_pointer, &resolver);
    if (resolverResult < 0 || !resolver)
    {
        FailStatus(error, "Failed to get managed function resolver", resolverResult);
        return std::nullopt;
    }
    auto state = std::make_shared<State>();
    state->loadAssembly = reinterpret_cast<load_assembly_fn>(loader);
    state->getFunction = reinterpret_cast<get_function_pointer_fn>(resolver);
    return Runtime(std::move(state));
}

bool Runtime::LoadAssembly(const std::filesystem::path& assembly, std::string* error) const
{
    ClearError(error);
    if (!m_State) return Fail(error, "Runtime has been moved from");
    if (assembly.empty()) return Fail(error, "IL assembly path is empty");
    const auto path = std::filesystem::absolute(assembly).native();
    const auto result = m_State->loadAssembly(path.c_str(), nullptr, nullptr);
    return result >= 0 || FailStatus(error, "Failed to load IL assembly", result);
}

void* Runtime::ResolveFunction(std::string_view typeName, std::string_view methodName, std::string* error) const
{
    ClearError(error);
    if (!m_State)
    {
        Fail(error, "Runtime has been moved from");
        return nullptr;
    }
    if (typeName.empty() || methodName.empty() || typeName.find('\0') != std::string_view::npos ||
        methodName.find('\0') != std::string_view::npos)
    {
        Fail(error, "Managed type and method names must be nonempty and contain no null bytes");
        return nullptr;
    }
    const auto type = HostString(typeName);
    const auto method = HostString(methodName);
    void* function = nullptr;
    const auto result = m_State->getFunction(type.c_str(), method.c_str(),
        UNMANAGEDCALLERSONLY_METHOD, nullptr, nullptr, &function);
    if (result < 0 || !function)
    {
        FailStatus(error, "Failed to resolve managed method " + std::string(typeName) + "::" + std::string(methodName), result);
        return nullptr;
    }
    return function;
}

bool Runtime::RegisterNativeFunction(std::string_view typeName, std::string_view registrationMethod,
                                     void* function, std::string* error) const
{
    ClearError(error);
    if (!function) return Fail(error, "Cannot register a null native function");
    using RegisterFunction = int32_t(void*);
    const auto registration = LoadFunction<RegisterFunction>(typeName, registrationMethod, error);
    if (!registration) return false;
    const auto result = registration(function);
    return result == 0 || FailStatus(error, "Managed native function registration failed", result);
}
}
