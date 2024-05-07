#pragma once
#include "../Render/Camera.h"
#include "Aether/Core/Math.h"
#include "Aether/Render/Camera.h"
namespace Aether {
class PerspectiveCameraController
{
public:
    PerspectiveCameraController() = default;

    void OnUpdate(float ds, PerspectiveCamera& camera);

private:
    float m_Speed = 3;
    float m_RotateSpeed = Math::PI / 16;
};
} // namespace Aether