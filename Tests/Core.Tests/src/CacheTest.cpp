#include "Core/Cache.h"
#include "doctest/doctest.h"
using namespace Aether;


TEST_CASE("LRUCache test")
{
    LRUCache<int, int> cache(2);
    cache.Put(1, 1);
    cache.Put(2, 2);
    CHECK(cache.Get(1).value() == 1);
    cache.Put(3, 3);
    CHECK(cache.Get(2).has_value() == false);
    cache.Put(4, 4);
    CHECK(cache.Get(1).has_value() == false);
    CHECK(cache.Get(3).value() == 3);
    CHECK(cache.Get(4).value() == 4);
}

struct CustomKey
{
    std::string tag;
    bool operator==(const CustomKey& other) const
    {
        return tag==other.tag;
    }
};
namespace Aether
{

template<>
struct LRUCacheHash<CustomKey>
{
    size_t operator()(const CustomKey& key) const
    {
        return std::hash<std::string>()(key.tag);
    }
};
}
TEST_CASE("LRUCache CustomKey")
{
    LRUCache<CustomKey, int> cache(2);
    cache.Put({"key1"}, 1);
    cache.Put({"key2"}, 2);
    CHECK(cache.Get({"key1"}).value() == 1);
    cache.Put({"key3"}, 3);
    CHECK(cache.Get({"key2"}).has_value() == false);
    cache.Put({"key4"}, 4);
    CHECK(cache.Get({"key1"}).has_value() == false);
    CHECK(cache.Get({"key3"}).value() == 3);
    CHECK(cache.Get({"key4"}).value() == 4);

}

TEST_CASE("LRUCache large capacity")
{
    LRUCache<int, int> cache(1000);
    for (int i = 0; i < 1000; i++)
    {
        cache.Put(i, i);
    }
    for (int i = 0; i < 1000; i++)
    {
        CHECK(cache.Get(i).value() == i);
    }
    cache.Put(1000, 1000);
    CHECK(cache.Get(0).has_value() == false);
    CHECK(cache.Get(1000).value() == 1000);
    cache.Put(1001, 1001);
    CHECK(cache.Get(1).has_value() == false);
    CHECK(cache.Get(1001).value() == 1001);
    cache.Put(1002, 1002);
    CHECK(cache.Get(2).has_value() == false);
    CHECK(cache.Get(1002).value() == 1002);
}
