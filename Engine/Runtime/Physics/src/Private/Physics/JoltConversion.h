#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Math/Vec3.h>
#include "Core/Math.h"

namespace Aether::Physics
{

inline JPH::Vec3 ToJolt(const Vec3f& value)
{
    return {value.x(), value.y(), value.z()};
}

inline JPH::RVec3 ToJoltPosition(const Vec3d& value)
{
    return {value.x(), value.y(), value.z()};
}

inline JPH::Quat ToJolt(const Quatf& value)
{
    return {value.x(), value.y(), value.z(), value.w()};
}

inline Vec3f FromJolt(JPH::Vec3Arg value)
{
    return {value.GetX(), value.GetY(), value.GetZ()};
}

inline Vec3d FromJoltPosition(JPH::RVec3Arg value)
{
    return {value.GetX(), value.GetY(), value.GetZ()};
}

inline Quatf FromJolt(JPH::QuatArg value)
{
    return {value.GetX(), value.GetY(), value.GetZ(), value.GetW()};
}

} // namespace Aether::Physics
