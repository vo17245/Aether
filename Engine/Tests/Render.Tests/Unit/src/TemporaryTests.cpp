#include "doctest/doctest.h"
#include "Render/Temporary.h"
using namespace Aether;

TEST_CASE("Test Temporary")
{
    Temporary<int,float,double> temp;
    temp.Push(new int(1));
    temp.Push(new float(1.0f));
    temp.Push(new double(1.0));
    auto& ints=temp.Get<int>();
    auto& floats=temp.Get<float>();
    auto& doubles=temp.Get<double>();
    CHECK(ints.size()==1);
    CHECK(floats.size()==1);
    CHECK(doubles.size()==1);
    CHECK(*ints[0]==1);
    CHECK(*floats[0]==1.0f);
    CHECK(*doubles[0]==1.0);
    temp.Clear<int>();
    CHECK(ints.size()==0);
    CHECK(floats.size()==1);
    CHECK(doubles.size()==1);
    temp.ClearAll();
    CHECK(ints.size()==0);
    CHECK(floats.size()==0);
    CHECK(doubles.size()==0);
    
}