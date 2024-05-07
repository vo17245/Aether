#pragma once
#include "Panel.h"
#include "Aether/Message.h"
#include <unordered_map>
#include "UI/UICommandHandler.h"
namespace Aether {
namespace Editor {
class EntityPanel : public Panel
{
public:
    EntityPanel()
    {
        RegisterUICommand();
    };
    ~EntityPanel(){

    };
    void OnImGuiRender() override;
    void OnUpdate(Real ds) override;
    void RegisterUICommand();

private:
    UICommandHandler m_CommandHandler;
};

}
} // namespace Aether::Editor