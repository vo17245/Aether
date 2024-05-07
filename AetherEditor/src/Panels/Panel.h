#pragma once
#include "Aether/Core/Math.h"
#include "Aether/Event.h"

namespace Aether {
namespace Editor {
class Panel
{
public:
    Panel() = default;
    virtual ~Panel() = default;
    virtual void OnImGuiRender(){};
    virtual void OnRender()
    {}
    virtual void OnEvent(Event& e)
    {}
    virtual void OnUpdate(Real ds)
    {}
};
}
} // namespace Aether::Editor