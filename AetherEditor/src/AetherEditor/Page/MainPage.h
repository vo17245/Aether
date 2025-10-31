#pragma once
#include "Page.h"
#include <AetherEditor/Panel/ContentBrowser.h>
namespace AetherEditor::UI
{

class MainPage : public Page
{
public:
    MainPage();
    void OnImGuiUpdate() override;
private:
    ContentBrowser m_ContentBrowser;
};
}