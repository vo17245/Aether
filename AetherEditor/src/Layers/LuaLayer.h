#pragma once

#include "Aether/Core/Layer.h"
namespace Aether {
namespace Editor {
class LuaLayer : public Layer
{
public:
    LuaLayer();
    ~LuaLayer();
    void OnUpdate(float ds) override;
    void OnEvent(Event& e) override;
    void OnRender() override;
    void OnImGuiRender() override;
};
}
} // namespace Aether::Editor