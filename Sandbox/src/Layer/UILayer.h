#pragma once

#include "Aether/Core/Layer.h"
#include "Aether/Message.h"
#include "Aether/Message/SubscribeReclaimer.h"
#include "Test/TestMenu.h"
#include "TestLayer.h"
#include <memory>
#include "Aether/Core.h"
#include "Test/TestRegister.h"
#include "Aether/Message.h"

namespace Aether
{
    class TestLayerTriggleMessage:public Message {};
    class SceneLayerTriggleMessage:public Message {};
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
        void OnTestLayerTriggle(Message* data)
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
       
        SubscribeReclaimer m_Reclaimer;
    };
}