#pragma once

#include "Aether/Core/Layer.h"
#include "Test/TestMenu.h"
#include "TestLayer.h"
#include <memory>
#include "Aether/Message/MessageBus.h"
#include "Aether/Core/Application.h"
namespace Aether
{
    class TestLayerTriggleMessage {};
    class UILayer:public Layer
    {
    public:
        UILayer();
        ~UILayer();
        void OnImGuiRender() override;
        void OnUpdate(float sec)override;
        void OnRender()override;
    private:
        Ref<TestLayer> m_TestLayer;
        void OnTestLayerTriggle(void* data)
        {
            if (m_TestLayer)
            {
                Application::Get().PopLayer(m_TestLayer);
                m_TestLayer.reset();
            }
            else
            {
                m_TestLayer = CreateRef<TestLayer>();
                Application::Get().PushLayer(m_TestLayer);
            }
        }
        std::vector<MessageBus::CallbackSignature> m_CallbackSignatures;
    };
}