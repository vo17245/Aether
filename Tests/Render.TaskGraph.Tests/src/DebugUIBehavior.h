#pragma once
#include <UI/Debug/DebugPage.h>
#include "LayerStack.h"
#include "TestTaskGraphCompile.h"
using namespace Aether;
class DebugUIBehavior : public UI::IDebugPageBehavior
{
public:
    virtual void OnInit(UI::DebugPage& page)
    {
        auto& hierarchy = page.GetHierarchy();
        {
            auto* node = hierarchy.GetNodeById("btn:Test TaskGraph Compile");
            auto& mc = node->AddComponent<UI::MouseComponent>();
            mc.onClick = [this]() {
                m_LayerStack->SetFirstLayer(CreateScope<TestTaskGraphCompile>());
            };
        }
        {
            m_Root=hierarchy.GetNodeById("root");
        }
    }
    DebugUIBehavior(Borrow<LayerStack> layerStack) :
        m_LayerStack(layerStack)
    {
    }
    virtual void OnEvent(UI::DebugPage& page, Event& event)
    {
        if (std::holds_alternative<KeyboardReleaseEvent>(event))
        {
            auto& e = std::get<KeyboardReleaseEvent>(event);
            if (KeyboardCode::KEY_ESCAPE == e.GetCode())
            {
                ToggleRootVisible();
            }
        }
    }

private:
    void ToggleRootVisible()
    {
        if(!m_Root)
        {
            assert(false);
            return;
        }
        auto& vc=m_Root->GetComponent<UI::VisibilityRequestComponent>();
        vc.processed=false;
        vc.visible=!vc.visible;
    }
    Borrow<LayerStack> m_LayerStack;
    UI::Node* m_Root;
};