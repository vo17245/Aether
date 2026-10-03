# Script

Script 提供 .NET IL 程序集加载、托管函数调用和 native 回调注册。
包含 `<Script/Script.h>`，链接 `Script` 或 `Aether::Script`。
安装后可以使用 `find_package(Script CONFIG REQUIRED)` 和 `Aether::Script`。

## 配置与构建

在 `Engine/CMake/Local.cmake` 配置依赖根目录：

```cmake
set(AETHER_DEPENDENCIES_DIR "/Users/vo17245/dev/AetherDependencies")
set(AETHER_DEPENDENCIES_PACKAGES_DIR "${AETHER_DEPENDENCIES_DIR}/Packages/Debug")
```

macOS arm64 的 dotnet root 固定由 `${AETHER_DEPENDENCIES_DIR}/Bin/mac-arm64/dotnet`
推导；其他支持的平台使用相应的 `Bin/<平台>-<架构>/dotnet`。
CMake 从该 SDK 查找 hostfxr 和 hosting 头文件，并将绝对路径写入模块内部配置。
同时从 `shared/Microsoft.NETCore.App` 选择最新稳定运行时版本，生成并嵌入 runtimeconfig JSON。
运行时不依赖 `PATH` 或环境变量 `DOTNET_ROOT`。
`Local.cmake` 是被 Git 忽略的本机配置文件，需要在每台机器上设置。

```sh
cmake -S Engine -B Engine/Build -DAETHER_BUILD_SCRIPT=ON -DAETHER_BUILD_SCRIPT_TESTS=ON
cmake --build Engine/Build --target ScriptTests ScriptTemporaryConfigTests --config Debug
ctest --test-dir Engine/Build -C Debug -R '^Script' --output-on-failure
```

Script 默认启用，可以通过 `AETHER_BUILD_SCRIPT=OFF` 关闭。
`AETHER_BUILD_SCRIPT_TESTS` 默认跟随 `AETHER_BUILD_TESTS`，测试 fixture 使用 .NET 10 SDK。
也可独立验证模块，无需构建渲染等依赖：

```sh
cmake -S Engine/Tests/Script.Tests -B /tmp/aether-script-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/aether-script-tests
ctest --test-dir /tmp/aether-script-tests --output-on-failure
```

## C++ 调用

```cpp
#include <Script/Script.h>
#include <cstdint>
#include <stdexcept>

int32_t AETHER_SCRIPT_CALL NativeMultiply(int32_t a, int32_t b) noexcept
{
    return a * b;
}

void RunScript()
{
    std::string error;
    auto runtime = Aether::Script::Runtime::Create(&error);
    if (!runtime) throw std::runtime_error(error);
    if (!runtime->LoadAssembly("GameScripts.dll", &error)) throw std::runtime_error(error);

    auto add = runtime->LoadFunction<int32_t(int32_t, int32_t)>(
        "GameScripts.EntryPoints, GameScripts", "Add", &error);
    if (!add) throw std::runtime_error(error);
    const auto sum = add(19, 23);

    if (!runtime->RegisterNativeFunction("GameScripts.EntryPoints, GameScripts",
                                        "RegisterMultiply", &NativeMultiply, &error))
        throw std::runtime_error(error);
}
```

`LoadFunction<返回类型(参数类型...)>` 返回 `Function<签名>`，只提供有效性检查和调用。
它不公开 JIT 地址，也不能转换为函数指针。内部以 callable 保存调用实现，未来
可以替换成 AOT 或解释器后端而保留调用方接口。目前实现的后端是 hostfxr；尚未
实现非 JIT 平台后端。函数对象保留后端状态，可以在创建它的 `Runtime` 销毁后继续调用。
加载失败返回空函数对象并填充可选的 `error`；调用空对象抛出 `std::bad_function_call`。

## C# 入口与 native 注册

```csharp
using System.Runtime.InteropServices;

namespace GameScripts;

public static class EntryPoints
{
    [UnmanagedFunctionPointer(CallingConvention.Winapi)]
    private delegate int NativeBinary(int a, int b);
    private static NativeBinary? s_Multiply;

    [UnmanagedCallersOnly]
    public static int Add(int a, int b) => unchecked(a + b);

    [UnmanagedCallersOnly]
    public static int RegisterMultiply(IntPtr callback)
    {
        if (callback == IntPtr.Zero) return -1;
        try
        {
            s_Multiply = Marshal.GetDelegateForFunctionPointer<NativeBinary>(callback);
            return 0;
        }
        catch { return -2; }
    }

    [UnmanagedCallersOnly]
    public static int Multiply(int a, int b)
    {
        try { return s_Multiply?.Invoke(a, b) ?? -1; }
        catch { return -2; }
    }
}
```

当前后端要求托管入口是带 `[UnmanagedCallersOnly]` 的静态方法，类型名称为
`Namespace.Type, AssemblyName`。C++ 签名必须准确匹配 C# 的非托管 ABI：例如
`int32_t` 对应 `int`，指针对应 `IntPtr`；字符串需以 UTF-8 指针和字节长度等显式参数传递。
注册方法固定使用 `int Method(IntPtr callback)`，返回 0 表示成功。
native 回调在托管代码仍可能调用它时必须保持有效；C# 需持有转换后的 delegate。
托管入口必须自行处理异常，避免异常穿过 native 调用边界。

程序集加载到默认 AssemblyLoadContext。多个 Runtime 共享进程内的 CoreCLR；
销毁 Runtime 不会关闭 CoreCLR 或卸载程序集，当前模块不提供热重载或隔离卸载。
使用者无需提供、部署或传入 `.runtimeconfig.json`。`Runtime::Create()` 将内置 JSON
写入唯一临时目录，在 hostfxr 初始化及 delegate 获取完成、初始化上下文关闭后，删除文件
和目录。POSIX 使用 `mkdtemp` 创建仅当前用户可访问的目录。框架版本取自构建时的 dotnet root，
`LatestPatch` 允许同一主次版本内更新补丁；加载的程序集需与所选运行时兼容。

临时目录定位、目录或文件创建、文件写入和关闭，以及文件或目录清理发生错误时，模块会
向 `stderr` 打印错误、刷新输出，然后立即 `std::abort()`。hostfxr 初始化、程序集加载和
函数解析错误仍通过返回值与可选的 `error` 返回；初始化失败时也会清理已创建的临时配置。
程序集继续使用 framework-dependent 部署；这一配置机制属于当前 hostfxr 后端内部实现。

实现参考 [Microsoft 的 .NET native hosting 文档](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)。
