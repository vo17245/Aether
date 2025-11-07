#pragma once
#include "Page.h"
#include <AetherEditor/Panel/Panel.h>
namespace AetherEditor::UI
{

    class  AssetPage:public Page
    {
    public:
        AssetPage():Page("AssetPage")
        {

        }
        Panel* PushPanel(Scope<Panel>&& panel)
        {
            m_OpenedPanels.push_back(std::move(panel));
            return m_OpenedPanels.back().get();
        }
        void SetActivePanel(Panel* panel)
        {
            m_ActivePanel = panel;
        }
        void PopPanel(Panel* panel)
        {
            if(m_ActivePanel==panel)
            {
                m_ActivePanel=nullptr;
            }
            auto it = std::find_if(m_OpenedPanels.begin(),m_OpenedPanels.end(),
                [panel](const Scope<Panel>& p)
                {
                    return p.get() == panel;
                });
            if(it!=m_OpenedPanels.end())
            {
                m_OpenedPanels.erase(it);
            }
        }
        void OnImGuiUpdate()override 
        {
            if(m_ActivePanel)
            {
                m_ActivePanel->OnImGuiUpdate();
            }
        }
    private:
        std::vector<Scope<Panel>> m_OpenedPanels;
        Panel* m_ActivePanel=nullptr;
    };
}