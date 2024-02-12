#include "TestLayer.h"

namespace Aether
{
    void TestLayer::OnImGuiRender()
    {
        if (m_TestMenu)
        {
            m_TestMenu->OnImGuiRender();

        }
    }
    void TestLayer::OnUpdate(float sec)
    {
        if (m_TestMenu)
        {
            m_TestMenu->OnUpdate(sec);
        }
    }
    void TestLayer::OnRender()
    {
        if (m_TestMenu)
        {
            m_TestMenu->OnRender();
        }
    }
    void TestLayer::OnEvent(Event& event)
    {
        if (m_TestMenu)
        {
            m_TestMenu->OnEvent(event);
        }
    }
}