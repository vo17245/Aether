#include "Core/String.h"
#include "Filesystem/FilesystemApi.h"
#include <print>
#include "Filesystem/Utils.h"
#include "doctest/doctest.h"
#include "Filesystem/Filesystem.h"
using namespace Aether;
TEST_CASE("Filesystem Find Test")
{
    Filesystem::CreateDirectories("tmp/Filesystem/FindTest");
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test1.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test2.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test3.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/test4.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/FindTest/中文.txt", std::span<uint8_t>());
    std::vector<U8String> files;
    
    auto res = Filesystem::FindFirst("tmp/Filesystem/FindTest/*");
    CHECK(res.has_value());
    while(res)
    {
        files.push_back(res->findData.name);
        res = Filesystem::FindNext(res->handle);
    }
    CHECK(files.size() == 7);
}

TEST_CASE("Filesystem ListFiles")
{
    Filesystem::CreateDirectories("tmp/Filesystem/ListFiles");
    Filesystem::WriteFile("tmp/Filesystem/ListFiles/test1.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/ListFiles/test2.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/ListFiles/test3.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/ListFiles/test4.txt", std::span<uint8_t>());
    Filesystem::WriteFile("tmp/Filesystem/ListFiles/中文.txt", std::span<uint8_t>());
    auto files = Filesystem::ListFiles("tmp/Filesystem/ListFiles");
    CHECK(files.size() == 5);
}