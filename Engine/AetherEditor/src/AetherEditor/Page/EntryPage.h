#pragma once
#include "Page.h"
namespace AetherEditor::UI
{

class EntryPage : public Page
{
public:
    EntryPage():Page("EntryPage")
    {
    }
    void OnImGuiUpdate() override;
};
}