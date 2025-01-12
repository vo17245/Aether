
#include "doctest/doctest.h"
#include "Render/Shader/Preprocessor.h"
#include <cstdio>
using namespace Aether;
static const char* funcH=R"(
void func()
{
    print("hello");
}
)";
static const char* mainCpp=R"(
#include <func.h>
)";
TEST_CASE("Test Preprocessor")
{
    Shader::Preprocessor preprocessor;
    preprocessor.AddInclude("tmp/Render/Test");
}