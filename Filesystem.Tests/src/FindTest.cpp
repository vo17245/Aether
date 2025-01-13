#include "Core/String.h"
#include "Filesystem/FilesystemApi.h"
#include <print>
#include "Filesystem/Utils.h"
#include "doctest/doctest.h"
#include "Filesystem/Filesystem.h"
using namespace Aether;
TEST_CASE("Filesystem Find Test")
{
    Filesystem::CreateDirectory("tmp/Filesystem");
    Filesystem::CreateDirectory("tmp/Filesystem/FindTest");
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test1.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test2.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test3.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test4.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/中文.txt", std::span<uint8_t>());
    auto res = Filesystem::FindFirst("tmp/Filesystem/FindTest");
    CHECK(res.has_value());
    res=Filesystem::FindNext(res->handle);
    CHECK(res.has_value());
    res=Filesystem::FindNext(res->handle);
    CHECK(res.has_value());
    res=Filesystem::FindNext(res->handle);
    CHECK(res.has_value());
    res=Filesystem::FindNext(res->handle);
    CHECK(res.has_value());
}