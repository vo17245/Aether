#include "Core/String.h"
#include "Filesystem/FilesystemApi.h"
#include <print>
#include "doctest/doctest.h"
#include "Filesystem/Filesystem.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
using namespace Aether;

TEST_CASE("Filesystem Path Parent")
{
    Filesystem::Path path("tmp/test.txt");
    auto parent = path.Parent();
    CHECK(parent.GetStr() == "tmp");
}