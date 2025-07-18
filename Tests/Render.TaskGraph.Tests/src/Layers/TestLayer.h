#pragma once
#include <Window/Layer.h>
#include "Tests/TestTaskGraphCompile.h"
#include "Tests/Test.h"
class TestLayer:public Layer
{
public:
    void SetTest(Scope<Test>&& test)
    {
        test->OnAttach(m_Window);
        m_Test = std::move(test);
    }
    virtual void OnAttach(Window* window)
    {
        m_Window = window;
        SetTest(CreateScope<TestTaskGraphCompile>());
    }
    virtual void OnDetach()
    {
        if (m_Test)
        {
            m_Test->OnDetach();
        }
    }
    virtual void OnRender(vk::RenderPass& renderPass,
                          vk::FrameBuffer& framebuffer,
                          vk::GraphicsCommandBuffer& commandBuffer)
    {
        if (m_Test)
        {
            m_Test->OnRender(renderPass, framebuffer, commandBuffer);
        }
    }

private:
    Scope<Test> m_Test;
    Window* m_Window = nullptr;
};