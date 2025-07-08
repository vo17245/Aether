#pragma once
#include <cmath>
namespace Aether
{
struct alignas(16) Mat2x3f
{
    float m00, m01, m02, _pad0; // 第一列
    float m10, m11, m12, _pad1; // 第二列
    void SetPosition(float x,float y)
    {
        m02 = x;
        m12 = y;
    }
    void SetScale(float x, float y)
    {
        m00 = x;
        m11 = y;
    }
    void SetRotation(float angle)
    {
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        m00 = cosAngle;
        m01 = -sinAngle;
        m10 = sinAngle;
        m11 = cosAngle;
    }
}; // 总共 32 字节
} // namespace Aether