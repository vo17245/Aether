#pragma once

#include "Panel.h"
#include "Panels/Panel.h"
#include "Aether/Message.h"
namespace Aether {
namespace Editor {
class LuaCameraScriptFileSelectPanel : public Panel
{
public:
    LuaCameraScriptFileSelectPanel();
    ~LuaCameraScriptFileSelectPanel(){}
    void OnImGuiRender() override;

private:
    bool m_Show = false;
    SubscribeReclaimer m_Reclaimer;
    std::string m_FilePath;
};
}
} // namespace Aether::Editor