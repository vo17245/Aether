#include "TemporaryRuntimeConfig.h"
#include <Script/Script.h>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>

#if !defined(_WIN32)
#include <sys/resource.h>
#endif

int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    const std::string_view mode = argv[1];
    // Give the test harness a deterministic exit status when abort() raises
    // SIGABRT, without generating core dumps or invoking the crash reporter.
    std::signal(SIGABRT, [](int) { std::_Exit(86); });

#if !defined(_WIN32)
    if (mode == "write-failure")
    {
        const rlimit limit{0, 0};
        if (setrlimit(RLIMIT_FSIZE, &limit) != 0) return 3;
        std::signal(SIGXFSZ, SIG_IGN);
    }
#endif

    if (mode == "create-failure" || mode == "write-failure")
    {
        std::string error;
        Aether::Script::Runtime::Create(&error);
        std::cerr << "Expected Runtime::Create to abort: " << error << '\n';
        return 6;
    }

    using Aether::Script::Detail::TemporaryRuntimeConfig;
    std::filesystem::path path;
    std::filesystem::path directory;
    {
        TemporaryRuntimeConfig config("{\"runtimeOptions\":{}}");
        path = config.Path();
        directory = path.parent_path();
        if (mode == "normal")
        {
            std::ifstream input(path, std::ios::binary);
            const std::string contents((std::istreambuf_iterator<char>(input)), {});
            if (contents != "{\"runtimeOptions\":{}}") return 4;
            TemporaryRuntimeConfig second("{}");
            if (second.Path() == path) return 5;
        }
        else if (mode == "file-cleanup-failure")
        {
            std::filesystem::remove(path);
            std::filesystem::create_directory(path);
            std::ofstream(path / "occupied") << "prevent removal";
        }
        else if (mode == "directory-cleanup-failure")
        {
            std::ofstream(directory / "occupied") << "prevent removal";
        }
        else
        {
            // create-failure and write-failure must already have aborted.
            return 6;
        }
    }
    if (mode != "normal") return 7;
    if (std::filesystem::exists(path) || std::filesystem::exists(directory)) return 8;
    std::cout << "Temporary runtime config tests passed\n";
    return 0;
}
