#include "Core/String.h"
#include "Filesystem/FilesystemApi.h"
#include <print>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Filesystem/Filesystem.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
using namespace Aether;
TEST_CASE("Filesystem create file")
{
    Filesystem::Path path("tmp/test.txt");
    Filesystem::Path tmpDir("tmp");
    if (!Filesystem::Exists(tmpDir))
    {
        Filesystem::CreateDirectory(tmpDir);
    }
    Filesystem::FileHandle handle;
    auto res = Filesystem::OpenFile(path, Filesystem::Action::Create, handle);
    CHECK(res == true);
    Filesystem::CloseFile(handle);
}

TEST_CASE("Filesystem create file chinese path")
{
    Filesystem::Path path("tmp/中文路径/test.txt");
    Filesystem::Path tmpDir("tmp");
    if (!Filesystem::Exists(tmpDir))
    {
        Filesystem::CreateDirectory(tmpDir);
    }
    if(!Filesystem::Exists("tmp/中文路径"))
    {
        Filesystem::CreateDirectory("tmp/中文路径");
    }
    Filesystem::FileHandle handle;
    auto res = Filesystem::OpenFile(path, Filesystem::Action::Create, handle);
    CHECK(res == true);
    Filesystem::CloseFile(handle);
}