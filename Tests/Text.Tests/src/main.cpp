#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <Text/Layout/Run.h>
#include <Core/Core.h>
using namespace Aether;
TEST_CASE("Test Split Runs")
{
    std::string text= "Hello, 世界! مرحبا بالعالم!";
    U8String u8text(text);
    U32String u32text = u8text.ToU32String();
    auto runs = Text::SplitRunsByDirectionAndScript(u32text);
}