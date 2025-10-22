#pragma once
#include <Core/TypeTraits.h>
#include "Def.h"
#include "Utils.h"
namespace Aether
{

template <typename T>
    requires std::is_floating_point_v<T>
struct Rotator
{
public:
    using Real = T;

    /** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
    T Pitch;

    /** Rotation around the up axis (around Z axis), Turning around (0=Forward, +Right, -Left)*/
    T Yaw;

    /** Rotation around the forward axis (around X axis), Tilting your head, (0=Straight, +Clockwise, -CCW) */
    T Roll;

public:
    /** A rotator of zero degrees on each axis. */
    static const Rotator<T> ZeroRotator;

public:
    Vec3<T> Forward() const
    {
        T CP = std::cos(Math::DegToRad(Pitch));
        T SP = std::sin(Math::DegToRad(Pitch));
        T CY = std::cos(Math::DegToRad(Yaw));
        T SY = std::sin(Math::DegToRad(Yaw));

        return Vec3<T>(CP * CY, CP * SY, SP);
    }
};
} // namespace Aether
