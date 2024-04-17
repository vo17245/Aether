#include "UILayer.h"
#include "Aether/Core/Application.h"
#include "Aether/ImGui.h"
#include "Aether/Core/Input.h"
#include "Aether/Core/Log.h"
#include "Layer/TestLayer.h"
namespace Aether
{
    UILayer::UILayer()
    {
        m_Reclaimer.Subscribe<TestLayerTriggleMessage>(
            AETHER_BIND_FN(OnTestLayerTriggle)
        );
    }
    UILayer::~UILayer()
    {
        
    }
    void Aether::UILayer::OnImGuiRender()
    {
    	ImGui::Begin("UILayer");
        if(ImGui::Button("TestLayer"))
        {
            MessageBus::GetSingleton().Publish<TestLayerTriggleMessage>(nullptr);
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