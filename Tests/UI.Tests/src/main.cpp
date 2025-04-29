#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "UI/Render/Renderer.h"
#include "Render/Config.h"
using namespace Aether;

TEST_CASE("Test Renderer::CalculateModelMatrix")
{
    UI::Renderer renderer=UI::Renderer::CreateEmpty();
    renderer.GetCamera().screenSize=Vec2f(1920,1080);
    renderer.GetCamera().CalculateMatrix();
    auto m=renderer.GetCamera().matrix;
    Vec2f p(0,1080);
    auto ph=Vec4f(p.x(),p.y(),0,1);
    auto ph2=m*ph;
    p=Vec2f(ph2.x(),ph2.y());
    CHECK(Render::Config::RenderApi==Render::Api::Vulkan);//if default api is not vulkan, this test should be updated
    CHECK(p==Vec2f(-1,-1));//in default render api(vulkan) ndc space
}
