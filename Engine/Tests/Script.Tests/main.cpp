#include <Script/Script.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace
{
int s_CallbackCount = 0;
std::filesystem::path Utf8Path(std::string_view value)
{
    return std::filesystem::path(std::u8string(value.begin(), value.end()));
}

bool HasTemporaryConfig(const std::filesystem::path& root)
{
    // CoreCLR also uses the temp root for diagnostic sockets while running.
    for (const auto& entry : std::filesystem::directory_iterator(root))
        if (entry.path().filename().string().starts_with("aether-script-")) return true;
    return false;
}

int32_t AETHER_SCRIPT_CALL Multiply(int32_t left, int32_t right) noexcept
{
    ++s_CallbackCount;
    return left * right;
}

void Check(bool condition, const char* expression, const std::string& error)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << expression << "\n" << error << '\n';
        std::exit(1);
    }
}
}

#define CHECK(expression) Check(static_cast<bool>(expression), #expression, error)

int main(int argc, char** argv)
{
    if (argc != 4) return 2;
    using Aether::Script::Runtime;
    using Binary = int32_t(int32_t, int32_t);
    constexpr auto type = "ScriptFixture.EntryPoints, ScriptFixture";
    std::string error;
    static_assert(!std::is_convertible_v<Aether::Script::Function<Binary>, void*>);
    static_assert(!std::is_convertible_v<Aether::Script::Function<Binary>, int32_t(*)(int32_t, int32_t)>);

    CHECK(Runtime::GetDotnetRoot() == Utf8Path(argv[2]));
    const auto temporaryRoot = Utf8Path(argv[3]);
    CHECK(!HasTemporaryConfig(temporaryRoot));
    auto configPath = Utf8Path(argv[1]);
    configPath.replace_extension(".runtimeconfig.json");
    CHECK(!std::filesystem::exists(configPath));
    auto runtime = Runtime::Create(&error);
    CHECK(runtime);
    CHECK(error.empty());
    CHECK(!HasTemporaryConfig(temporaryRoot));
    CHECK(!runtime->LoadAssembly({}, &error));
    CHECK(!error.empty());
    CHECK(!runtime->LoadAssembly(Utf8Path(argv[1]).parent_path() / "missing.dll", &error));
    CHECK(!error.empty());
    CHECK(runtime->LoadAssembly(Utf8Path(argv[1]), &error));
    CHECK(error.empty());

    auto add = runtime->LoadFunction<Binary>(type, "Add", &error);
    CHECK(add);
    CHECK(add(19, 23) == 42);
    CHECK(add(-10, 5) == -5);
    auto notify = runtime->LoadFunction<void()>(type, "Notify", &error);
    auto count = runtime->LoadFunction<int32_t()>(type, "Count", &error);
    CHECK(notify && count);
    notify();
    notify();
    CHECK(count() == 2);
    auto utf8Length = runtime->LoadFunction<int32_t(const char*, int32_t)>(type, "Utf8Length", &error);
    CHECK(utf8Length);
    const char message[] = "Hello, 你好";
    CHECK(utf8Length(message, sizeof(message) - 1) == 9);

    auto callNative = runtime->LoadFunction<Binary>(type, "CallNative", &error);
    CHECK(callNative);
    CHECK(callNative(6, 7) == -1);
    CHECK(!runtime->RegisterNativeFunction(type, "RegisterNative", nullptr, &error));
    CHECK(!error.empty());
    CHECK(!runtime->RegisterNativeFunction(type, "RejectNative", &Multiply, &error));
    CHECK(error.find("ffffffef") != std::string::npos);
    CHECK(runtime->RegisterNativeFunction(type, "RegisterNative", &Multiply, &error));
    CHECK(error.empty());
    CHECK(callNative(6, 7) == 42);
    CHECK(s_CallbackCount == 1);

    CHECK(!runtime->LoadFunction<int32_t()>(type, "MissingMethod", &error));
    CHECK(!error.empty());
    CHECK(!runtime->LoadFunction<int32_t()>("ScriptFixture.Missing, ScriptFixture", "Count", &error));
    CHECK(!error.empty());
    CHECK(!runtime->LoadFunction<int32_t()>(type, "OrdinaryMethod", &error));
    CHECK(!error.empty());
    CHECK(!runtime->LoadFunction<int32_t()>({}, "Count", &error));
    CHECK(!error.empty());

    // Host contexts can be recreated; callable values retain their backend
    // state after the originating Runtime is copied, moved, and destroyed.
    auto second = Runtime::Create(&error);
    CHECK(second);
    CHECK(!HasTemporaryConfig(temporaryRoot));
    CHECK(second->LoadAssembly(Utf8Path(argv[1]), &error));
    {
        auto copy = *runtime;
        auto moved = std::move(copy);
        CHECK(!copy.LoadFunction<Binary>(type, "Add", &error));
        CHECK(moved.LoadFunction<Binary>(type, "Add", &error)(1, 2) == 3);
    }
    runtime.reset();
    second.reset();
    CHECK(add(20, 22) == 42);
    CHECK(callNative(7, 8) == 56);
    CHECK(s_CallbackCount == 2);
    auto empty = Aether::Script::Function<void()>();
    CHECK(!empty);
    try { empty(); CHECK(false); }
    catch (const std::bad_function_call&) {}
    // Fresh contexts reset static state and keep escaped functions alive.
    auto reloadable = Runtime::CreateReloadable(&error);
    CHECK(reloadable);
    CHECK(reloadable->LoadAssembly(Utf8Path(argv[1]), &error));
    auto freshCount = reloadable->LoadFunction<int32_t()>(type, "Count", &error);
    auto freshNotify = reloadable->LoadFunction<void()>(type, "Notify", &error);
    CHECK(freshCount && freshNotify);
    CHECK(freshCount() == 0);
    freshNotify();
    CHECK(freshCount() == 1);
    CHECK(!reloadable->LoadFunction<int32_t()>(type, "OrdinaryMethod", &error));
    CHECK(!error.empty());
    reloadable.reset();
    CHECK(freshCount() == 1);
    auto isolated = Runtime::CreateReloadable(&error);
    CHECK(isolated && isolated->LoadAssembly(Utf8Path(argv[1]), &error));
    CHECK(isolated->LoadFunction<int32_t()>(type, "Count", &error)() == 0);
    CHECK(freshCount() == 1);
    std::cout << "Script integration tests passed\n";
    return 0;
}
