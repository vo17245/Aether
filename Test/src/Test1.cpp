#include <catch2/catch_amalgamated.hpp>
#include "Aether/Resource/Model/Buffer.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Aether/Resource/Model/Model.h"
using namespace Aether;

TEST_CASE("test Accessor iterator 1") {
    std::vector<float> arr{1,2,3,4,5,6};
    Model model;
    model.buffers.emplace_back(reinterpret_cast<std::byte*>(arr.data()),
        reinterpret_cast<std::byte*>(arr.data() + arr.size()));
    auto& buffer = model.buffers[0];
    model.bufferViews.emplace_back(0, 0, buffer.size());
    auto& bufferView = model.bufferViews[0];
    model.accessors.emplace_back(0, 0, ElementType::VEC3, &model);
    auto& accessor = model.accessors[0];
    auto iter = accessor.Begin();
    REQUIRE((*iter).vec3.x() == 1);
    REQUIRE((*iter).vec3.y() == 2);
    REQUIRE((*iter).vec3.z() == 3);
    ++iter;
    REQUIRE((*iter).vec3.x() == 4);
    REQUIRE((*iter).vec3.y() == 5);
    REQUIRE((*iter).vec3.z() == 6);

}

TEST_CASE("test Accessor iterator 2") {
    std::vector<float> arr{ 1,2,3,4,5,6 };
    Model model;
    model.buffers.emplace_back(reinterpret_cast<std::byte*>(arr.data()),
        reinterpret_cast<std::byte*>(arr.data() + arr.size()));
    auto& buffer = model.buffers[0];
    model.bufferViews.emplace_back(0, 0, buffer.size());
    auto& bufferView = model.bufferViews[0];
    model.accessors.emplace_back(0, sizeof(Real)*3, ElementType::VEC2, &model);
    auto& accessor = model.accessors[0];
    auto iter = accessor.Begin();
    REQUIRE((*iter).vec2.x() == 1);
    REQUIRE((*iter).vec2.y() == 2);
    ++iter;
    REQUIRE((*iter).vec2.x() == 4);
    REQUIRE((*iter).vec2.y() == 5);
}

TEST_CASE("test Accessor iterator 3") {
    std::vector<float> arr{ 1,2,3,4,5,6 };
    Model model;
    model.buffers.emplace_back(reinterpret_cast<std::byte*>(arr.data()),
        reinterpret_cast<std::byte*>(arr.data() + arr.size()));
    auto& buffer = model.buffers[0];
    model.bufferViews.emplace_back(0, 0, buffer.size());
    auto& bufferView = model.bufferViews[0];
    model.accessors.emplace_back(0, sizeof(Real) * 3, ElementType::REAL, &model);
    auto& accessor = model.accessors[0];
    auto iter = accessor.Begin();
    REQUIRE((*iter).real == 1);
    ++iter;
    REQUIRE((*iter).real == 4);
  
}
