#pragma once
#include "Aether/Core/Layer.h"
#include "Test/TestMenu.h"
#include <memory>
#include "Test/TestRegister.h"
namespace Aether
{
    class TestLayer:public Layer
    {
    public:
        TestLayer()
        {
            m_TestMenu = Test::TestRegister::GetSingleton().CreateTestMenu();
        }
        ~TestLayer()=default;
        void OnImGuiRender() override;
        void OnUpdate(float sec)override;
        void OnRender()override;
        void OnEvent(Event& event)override;
    private:
        Scope<Test::TestMenu> m_TestMenu;
    };
}