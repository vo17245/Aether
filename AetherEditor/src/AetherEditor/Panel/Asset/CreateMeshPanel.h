#pragma once
#include <AetherEditor/Panel/Panel.h>
class CreateMeshPanel : public Panel
{
public:
    std::string& GetTitle() override
    {
    }
    void SetVisible(bool visible) override {};
    bool GetVisible() override {};
    void OnImGuiUpdate() override {};
    void SetPosition(float x, float y) override {};
    void SetSize(float width, float height) override {};

private:
    
};