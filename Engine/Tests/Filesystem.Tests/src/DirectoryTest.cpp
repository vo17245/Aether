#include "Core/String.h"
#include "Filesystem/FilesystemApi.h"
#include <print>
#include "doctest/doctest.h"
#include "Filesystem/Filesystem.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
using namespace Aether;

TEST_CASE("Filesystem::Directory::CreateDirectories")
{
    Filesystem::Path path("tmp/CreateDirectories/test2/test3/test4");
    auto res = Filesystem::Directory::CreateDirectories(path);
    CHECK(res == true);
    CHECK(Filesystem::Exists("tmp"));
    CHECK(Filesystem::Exists("tmp/CreateDirectories"));
    CHECK(Filesystem::Exists("tmp/CreateDirectories/test2"));
    CHECK(Filesystem::Exists("tmp/CreateDirectories/test2/test3"));
    CHECK(Filesystem::Exists("tmp/CreateDirectories/test2/test3/test4"));
    
}