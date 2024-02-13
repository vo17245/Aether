#include "UILayer.h"
#include "Aether/Core/Application.h"
#include "Aether/ImGui/ImGui"
#include "Aether/Core/Input.h"
#include "Aether/Core/Log.h"
#include "Layer/TestLayer.h"
namespace Aether
{
    UILayer::UILayer()
    {
        {
            auto signature = MessageBus::GetSingleton().Subscribe<TestLayerTriggleMessage>(
                AETHER_BIND_FN(OnTestLayerTriggle)
            );
            m_CallbackSignatures.emplace_back(signature);
        }
        
        {
            auto signature = MessageBus::GetSingleton().Subscribe<SceneLayerTriggleMessage>(
                AETHER_BIND_FN(OnSceneLayerTriggle)
            );
            m_CallbackSignatures.emplace_back(signature);
        }
        
        
    }
    UILayer::~UILayer()
    {
        for (auto& signature : m_CallbackSignatures)
        {
            MessageBus::GetSingleton().Unsubscribe(signature);
        }
    }
    void Aether::UILayer::OnImGuiRender()
    {
    	ImGui::Begin("UILayer");
        if(ImGui::Button("TestLayer"))
        {
            MessageBus::GetSingleton().Publish<TestLayerTriggleMessage>(nullptr);
        }
        if (ImGui::Button("SceneLayer"))
        {
            MessageBus::GetSingleton().Publish<SceneLayerTriggleMessage>(nullptr);
        }
    	ImGui::End();

    
    }
    void UILayer::OnUpdate(float sec)
    {
       
    }
    void UILayer::OnRender()
    {

    }
}