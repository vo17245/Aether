#pragma once

#include "Aether/Core/Layer.h"
#include "Aether/Message/Message.h"
#include "Test/TestMenu.h"
#include "TestLayer.h"
#include <memory>
#include "Aether/Message/MessageBus.h"
#include "Aether/Core/Application.h"
#include "Test/TestRegister.h"
#include "SceneLayer.h"
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
        Ref<SceneLayer> m_SceneLayer;
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
        void OnSceneLayerTriggle(Message* data)
        {
            if (m_SceneLayer)
            {
                Application::Get().PopLayer(m_SceneLayer);
                m_SceneLayer.reset();
            }
            else
            {
                m_SceneLayer = CreateRef<SceneLayer>();
                Application::Get().PushLayer(m_SceneLayer);
            }
        }
        std::vector<MessageBus::CallbackSignature> m_CallbackSignatures;
    };
}