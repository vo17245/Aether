#include "TestLayer.h"

namespace Aether
{
    void TestLayer::OnImGuiRender()
    {
            m_TestMenu.OnImGuiRender();
    }
    void TestLayer::OnUpdate(float sec)
    {
            m_TestMenu.OnUpdate(sec);
    }
    void TestLayer::OnRender()
    {
            m_TestMenu.OnRender();
    }
    void TestLayer::OnEvent(Event& event)
    {
            m_TestMenu.OnEvent(event);
    }
}