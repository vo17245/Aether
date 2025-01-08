
#include "Core/Base.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
using namespace Aether;
#include "Render/Shader/Lines.h"

TEST_CASE("Test Lines LF")
{
    std::string code = "line1\nline2\nline3\n";
    auto lines=Shader::Lines(code);
    auto iter=lines.begin();
    CHECK(iter!=lines.end());
    CHECK(iter=="line1");
    ++iter;
    CHECK(iter!=lines.end());
    CHECK(iter=="line2");
    ++iter;
    CHECK(iter!=lines.end());
    CHECK(iter=="line3");
    ++iter;
    CHECK(iter==lines.end());
}
TEST_CASE("Test Lines CRLF")
{
    std::string code = "line1\r\nline2\r\nline3\r\n";
    auto lines=Shader::Lines(code);
    auto iter=lines.begin();
    CHECK(iter!=lines.end());
    CHECK(iter=="line1");
    ++iter;
    CHECK(iter!=lines.end());
    CHECK(iter=="line2");
    ++iter;
    CHECK(iter!=lines.end());
    CHECK(iter=="line3");
    ++iter;
    CHECK(iter==lines.end());
}


