#include "TemporaryRuntimeConfig.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <system_error>

#if defined(_WIN32)
#include <random>
#else
#include <unistd.h>
#include <vector>
#endif

namespace Aether::Script::Detail
{
namespace
{
[[noreturn]] void Abort(std::string_view message) noexcept
{
    std::fprintf(stderr, "Script: temporary runtime config failure: %.*s\n",
                 static_cast<int>(message.size()), message.data());
    std::fflush(stderr);
    std::abort();
}

std::filesystem::path CreateDirectory(const std::filesystem::path& root)
{
#if defined(_WIN32)
    std::random_device random;
    for (int attempt = 0; attempt != 64; ++attempt)
    {
        const auto directory = root / ("aether-script-" + std::to_string(random()) + "-" + std::to_string(random()));
        std::error_code error;
        if (std::filesystem::create_directory(directory, error)) return directory;
        if (error && error != std::errc::file_exists)
            Abort("Cannot create temporary directory '" + directory.string() + "': " + error.message());
    }
    Abort("Cannot create a unique temporary directory");
#else
    // mkdtemp creates the directory atomically with owner-only permissions.
    const auto pattern = (root / "aether-script-XXXXXX").native();
    std::vector<char> name(pattern.begin(), pattern.end());
    name.push_back('\0');
    if (!::mkdtemp(name.data()))
    {
        const std::error_code error(errno, std::generic_category());
        Abort("Cannot create temporary directory in '" + root.string() + "': " +
              error.message());
    }
    return std::filesystem::path(name.data());
#endif
}
}

TemporaryRuntimeConfig::TemporaryRuntimeConfig(std::string_view contents)
{
    try
    {
        std::error_code error;
        const auto root = std::filesystem::temp_directory_path(error);
        if (error) Abort("Cannot locate temporary directory: " + error.message());
        m_Directory = CreateDirectory(root);
        m_Path = m_Directory / "Script.runtimeconfig.json";
        std::ofstream file(m_Path, std::ios::binary | std::ios::trunc);
        if (!file) Abort("Cannot create temporary file '" + m_Path.string() + "'");
        file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        if (!file) Abort("Cannot write temporary file '" + m_Path.string() + "'");
        file.close();
        if (!file) Abort("Cannot close temporary file '" + m_Path.string() + "'");
    }
    catch (const std::exception& error)
    {
        Abort(error.what());
    }
    catch (...)
    {
        Abort("Unknown error creating temporary runtime config");
    }
}

TemporaryRuntimeConfig::~TemporaryRuntimeConfig() noexcept
{
    try
    {
        std::error_code error;
        std::filesystem::remove(m_Path, error);
        if (error) Abort("Cannot remove temporary file '" + m_Path.string() + "': " + error.message());
        std::filesystem::remove(m_Directory, error);
        if (error) Abort("Cannot remove temporary directory '" + m_Directory.string() + "': " + error.message());
    }
    catch (const std::exception& error)
    {
        Abort(error.what());
    }
    catch (...)
    {
        Abort("Unknown error removing temporary runtime config");
    }
}
}
