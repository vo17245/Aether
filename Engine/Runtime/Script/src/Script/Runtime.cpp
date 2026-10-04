#include "Runtime.h"
#include "ScriptConfig.h"
#include "BridgeAssembly.h"
#include <mutex>
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
    void* session = nullptr;
    using Load = int32_t(AETHER_SCRIPT_CALL*)(void*, const char*, char*, int32_t);
    using Resolve = int32_t(AETHER_SCRIPT_CALL*)(void*, const char*, const char*, void**, char*, int32_t);
    using Release = void(AETHER_SCRIPT_CALL*)(void*);
    Load reloadableLoad = nullptr;
    Resolve reloadableResolve = nullptr;
    Release release = nullptr;
    ~State() { if (session && release) release(session); }
};

std::filesystem::path Runtime::GetDotnetRoot()
{
    constexpr std::string_view root = AETHER_SCRIPT_DOTNET_ROOT;
    return std::filesystem::path(std::u8string(root.begin(), root.end()));
}

std::string_view Runtime::GetTargetFramework() { return AETHER_SCRIPT_DOTNET_TFM; }

std::optional<Runtime> Runtime::CreateReloadable(std::string* error)
{
    auto host = Create(error);
    if (!host) return std::nullopt;
    // Bridge initialization is shared, while every returned Runtime gets a new context.
    static std::mutex mutex;
    static bool loaded = false;
    std::lock_guard lock(mutex);
    if (!loaded)
    {
        static const HostingApi api;
        const Detail::TemporaryRuntimeConfig config(Detail::RuntimeConfig);
        const auto root = GetDotnetRoot().native();
        const hostfxr_initialize_parameters parameters{sizeof(hostfxr_initialize_parameters), nullptr, root.c_str()};
        HostContext context{nullptr, api.close};
        auto status = api.initialize(config.Path().c_str(), &parameters, &context.handle);
        void* loader = nullptr;
        if (status >= 0 && context.handle)
            status = api.getDelegate(context.handle, hdt_load_assembly_bytes, &loader);
        if (status < 0 || !loader)
        {
            FailStatus(error, "Failed to get bridge loader", status);
            return std::nullopt;
        }
        status = reinterpret_cast<load_assembly_bytes_fn>(loader)(Detail::BridgeAssembly,
            sizeof(Detail::BridgeAssembly), nullptr, 0, nullptr, nullptr);
        if (status < 0)
        {
            FailStatus(error, "Failed to load Script bridge", status);
            return std::nullopt;
        }
        loaded = true;
    }
    constexpr auto type = "Aether.Script.Bridge, Aether.Script.Bridge";
    const auto create = host->LoadFunction<void*()>(type, "Create", error);
    if (!create) return std::nullopt;
    auto state = std::make_shared<State>();
    state->reloadableLoad = reinterpret_cast<State::Load>(host->ResolveFunction(type, "Load", error));
    if (!state->reloadableLoad) return std::nullopt;
    state->reloadableResolve = reinterpret_cast<State::Resolve>(host->ResolveFunction(type, "Resolve", error));
    if (!state->reloadableResolve) return std::nullopt;
    state->release = reinterpret_cast<State::Release>(host->ResolveFunction(type, "Release", error));
    if (!state->release) return std::nullopt;
    state->session = create();
    if (!state->session)
    {
        Fail(error, "Failed to create reloadable Script context");
        return std::nullopt;
    }
    return Runtime(std::move(state));
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
    if (m_State->session)
    {
        const auto utf8 = std::filesystem::absolute(assembly).u8string();
        char message[4096]{};
        const auto result = m_State->reloadableLoad(m_State->session,
            reinterpret_cast<const char*>(utf8.c_str()), message, sizeof(message));
        return result == 0 || Fail(error, "Failed to load IL assembly: " + std::string(message));
    }
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
    if (m_State->session)
    {
        void* function = nullptr;
        char message[4096]{};
        const auto result = m_State->reloadableResolve(m_State->session,
            std::string(typeName).c_str(), std::string(methodName).c_str(), &function, message, sizeof(message));
        if (result != 0) Fail(error, "Failed to resolve managed method: " + std::string(message));
        return function;
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
