#pragma once
#include "Aether/Core/Layer.h"
#include "Test/TestMenu.h"
#include <memory>
namespace Aether
{
    class TestLayer:public Layer
    {
    public:
        TestLayer()=default;
        ~TestLayer()=default;
        void OnImGuiRender() override;
        void OnUpdate(float sec)override;
        void OnRender()override;
        void OnEvent(Event& event)override;
    private:
        std::unique_ptr<TestMenu> m_TestMenu;
    };
}