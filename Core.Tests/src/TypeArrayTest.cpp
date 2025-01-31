#include "doctest/doctest.h"
#include "Core/TypeArray.h"
using namespace Aether;

TEST_CASE("Test TypeIndex")
{
    using Types=TypeArray<int,float,double>;
    CHECK(TypeIndexInArray<int, Types>::value==0);
    CHECK(TypeIndexInArray<float, Types>::value==1);
    CHECK(TypeIndexInArray<double, Types>::value==2);
}
TEST_CASE("Test IsContainType")
{
    using Types=TypeArray<int,float,double>;
    CHECK(IsArrayContainType<int,Types>::value==true);
    CHECK(IsArrayContainType<float,Types>::value==true);
    CHECK(IsArrayContainType<double,Types>::value==true);
    CHECK(IsArrayContainType<char,Types>::value==false);
}


