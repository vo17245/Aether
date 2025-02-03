#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Resource/Finder.h"

using namespace Aether::Resource;
TEST_CASE("Test Finder::Find")
{
    Finder finder;
    finder.AddBundleDir("Assets");
    auto result=finder.Find("Images/tiles.png");
    CHECK(result);
}