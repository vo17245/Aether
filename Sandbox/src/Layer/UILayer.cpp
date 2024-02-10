#include "UILayer.h"
#include "Aether/ImGui/ImGui"
namespace Aether
{
    UILayer::UILayer()
    {

    }
    void Aether::UILayer::OnImGuiRender()
    {
    	ImGui::Begin("Sandbox");
    	ImGui::Text("Hello");
    	ImGui::End();
    
    }
}