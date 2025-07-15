#pragma once
#include <cmath>
#include <Eigen/Core>
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
    Eigen::Matrix<float, 2, 3> ToEigen()const
    {
        Eigen::Matrix<float, 2, 3> mat;
        mat(0, 0) = m00;
        mat(0, 1) = m01;
        mat(0, 2) = m02;
        mat(1, 0) = m10;
        mat(1, 1) = m11;
        mat(1, 2) = m12;
        return mat;
    }
    static Mat2x3f FromEigen(const Eigen::Matrix<float, 2, 3>& mat)
    {
        Mat2x3f result;
        result.m00 = mat(0, 0);
        result.m01 = mat(0, 1);
        result.m02 = mat(0, 2);
        result.m10 = mat(1, 0);
        result.m11 = mat(1, 1);
        result.m12 = mat(1, 2);
        result._pad0 = 0.0f; // 填充
        result._pad1 = 0.0f; // 填充
        return result;
    }
    static Mat2x3f Identity()
    {
        Mat2x3f result;
        result.m00 = 1.0f;
        result.m01 = 0.0f;
        result.m02 = 0.0f;
        result.m10 = 0.0f;
        result.m11 = 1.0f;
        result.m12 = 0.0f;
        result._pad0 = 0.0f; // 填充
        result._pad1 = 0.0f; // 填充
        return result;
    }

}; // 总共 32 字节
} // namespace Aether