#pragma once
#include "Core/Math.h"
#include "../Basis.h"
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
namespace Aether::Render::Scene
{
struct Camera3D
{
    Vec3f position = {0, 0, 0};
    Vec3f target = {0, 0, -1};
    Vec3f up = {0, 1, 0};

    // 投影参数
    float fov = 45.0f;// in radians
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 1000.0f;

    Mat4f viewMatrix = Mat4f::Identity();
    Mat4f projMatrix = Mat4f::Identity();
    Mat4f viewProjMatrix = Mat4f::Identity();
    void CalculateMatrices()
    {
        viewMatrix = Math::LookAt(position, target, up);
        projMatrix = Math::Perspective(fov, aspect, near, far);
        viewProjMatrix = projMatrix * viewMatrix;
    }
};
} // namespace Aether