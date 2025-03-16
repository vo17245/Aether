#include "doctest/doctest.h"
#include "Core/LocalRef.h"
using namespace Aether;
#include <functional>
struct Entity
{
std::function<void()> m_OnDestroy=[](){};
Entity()
{
}
Entity(const std::function<void()>& onDestroy)
{
m_OnDestroy=onDestroy;
}
~Entity()
{
m_OnDestroy();
}
};

TEST_CASE("LocalRef test1")
{
    int isDestroyed=0;
    LocalRef<Entity> entity1;
    {
        auto entity=CreateLocalRef<Entity>([&](){
            isDestroyed=1;
        });
        entity1=std::move(entity);
    }
    CHECK(isDestroyed==0);
    
}
TEST_CASE("LocalRef test2")
{
    int isDestroyed=0;
    LocalRef<Entity> entity1;
    {
        auto entity=CreateLocalRef<Entity>([&](){
            isDestroyed=1;
        });
        entity1=std::move(entity);
    }
    CHECK(isDestroyed==0);
    entity1=LocalRef<Entity>();
    CHECK(isDestroyed==1);
}
TEST_CASE("LocalRef test3")
{
    int isDestroyed=0;
    LocalRef<Entity> entity1;
    {
        auto entity=CreateLocalRef<Entity>([&](){
            isDestroyed=1;
        });
        entity1=std::move(entity);
    }
    CHECK(isDestroyed==0);
    entity1=LocalRef<Entity>();
    CHECK(isDestroyed==1);
    entity1=CreateLocalRef<Entity>([&](){
        isDestroyed=2;
    });
    CHECK(isDestroyed==1);
}
TEST_CASE("LocalRef test4")
{
    int isDestroyed=0;
    LocalRef<Entity> entity1;
    {
        auto entity=CreateLocalRef<Entity>([&](){
            isDestroyed=1;
        });
        entity1=std::move(entity);
    }
    CHECK(isDestroyed==0);
    entity1=LocalRef<Entity>();
    CHECK(isDestroyed==1);
    entity1=CreateLocalRef<Entity>([&](){
        isDestroyed=2;
    });
    CHECK(isDestroyed==1);
    entity1=LocalRef<Entity>();
    CHECK(isDestroyed==2);
}
