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

    }
    void Aether::UILayer::OnImGuiRender()
    {
    	ImGui::Begin("Sandbox");
    	ImGui::Text("Hello");
        if(ImGui::Button("Test Menu"))
        {
            if(m_TestLayer)
            {
                Application::Get().PopLayer(m_TestLayer);
            }
            else 
            {
                m_TestLayer=CreateRef<TestLayer>();
                Application::Get().PushLayer(m_TestLayer);
            }
            
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