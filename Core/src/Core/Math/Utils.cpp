#include "Utils.h"
#include <Eigen/Geometry>
#include <Eigen/Dense>
namespace Aether {
namespace Math {

Eigen::Matrix4f Perspective(const float fovy, const float aspectRatio, const float zNear, const float zFar)
{
    const float tanHalfFovy = std::tan(fovy / 2.0f);
    Eigen::Matrix4f perspectiveMatrix;
    perspectiveMatrix.setZero();
    perspectiveMatrix(0, 0) = 1.0f / (aspectRatio * tanHalfFovy);
    perspectiveMatrix(1, 1) = -1.0f / tanHalfFovy;
    perspectiveMatrix(2, 2) = -(zFar + zNear) / (zFar - zNear);
    perspectiveMatrix(2, 3) = -2.0f * zFar * zNear / (zFar - zNear);
    perspectiveMatrix(3, 2) = -1.0f;

    return perspectiveMatrix;
}

Eigen::Matrix4f Translation(const Vec3& v)
{
    Eigen::Matrix4f m;
    m << 1, 0, 0, v.x(),
        0, 1, 0, v.y(),
        0, 0, 1, v.z(),
        0, 0, 0, 1;
    return m;
}

Eigen::Matrix4f Scale(const Vec3& v)
{
    Eigen::Matrix4f m;
    m << v.x(), 0, 0, 0,
        0, v.y(), 0, 0,
        0, 0, v.z(), 0,
        0, 0, 0, 1;
    return m;
}
Eigen::Matrix4f RotateX(float a)
{
    float sina = std::sin(a);
    float cosa = std::cos(a);
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,
        0, cosa, -1 * sina, 0,
        0, sina, cosa, 0,
        0, 0, 0, 1;
    return m;
}
Eigen::Matrix4f RotateY(float a)
{
    float sina = std::sin(a);
    float cosa = std::cos(a);
    Eigen::Matrix4f m;
    m << cosa, 0, sina, 0,
        0, 1, 0, 0,
        -1 * sina, 0, cosa, 0,
        0, 0, 0, 1;
    return m;
}
Eigen::Matrix4f RotateZ(float a)
{
    float sina = std::sin(a);
    float cosa = std::cos(a);
    Eigen::Matrix4f m;
    m << cosa, -1 * sina, 0, 0,
        sina, cosa, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    return m;
}
Eigen::Matrix4f Rotation(const Vec3& v)
{
    return RotateZ(v.z()) * RotateY(v.y()) * RotateX(v.x());
}
Eigen::Matrix4f Identity()
{
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    return m;
}
Eigen::Matrix4f Orthographic(float left, float right, float bottom, float top, float near, float far)
{
    Eigen::Matrix4f ortho;
    ortho << 2.0f / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, -2.0f / (far - near), -(far + near) / (far - near),
        0, 0, 0, 1.0f;
    return ortho;
}
Mat4f Rotation(const Vec3f& axis, const float angle)
{
    Eigen::AngleAxisf rotation(angle, axis);
    Eigen::Matrix3f rotationMatrix = rotation.toRotationMatrix();
    Mat4f mat = Mat4f::Identity();
    for (size_t row = 0; row < 3; row++)
    {
        for (size_t col = 0; col < 3; col++)
        {
            mat(row, col) = rotationMatrix(row, col);
        }
    }
    return mat;
}
Mat3f GetRotation(const Vec3f& a, const Vec3f& b)
{
    // Compute rotation axis
    Eigen::Vector3f axis = a.cross(b);
    if (axis.norm() == 0)
    {
        axis = Eigen::Vector3f(1, 0, 0);
    }
    else
    {
        axis.normalize();
    }

    // Compute rotation angle
    float angle = acos(a.dot(b) / (a.norm() * b.norm()));

    // Construct rotation matrix
    Eigen::Matrix3f rotation_matrix;
    rotation_matrix = Eigen::AngleAxisf(angle, axis).toRotationMatrix();
    return rotation_matrix;
}
Mat4f Homogeneous(const Mat3f& m)
{
    // Create a 4x4 matrix from the 3x3 matrix
    Eigen::Matrix4f Mat4f = Eigen::Matrix4f::Identity();
    Mat4f.block<3, 3>(0, 0) = m;
    return Mat4f;
}
Eigen::Matrix4d Scaled(const Vec3d& v)
{
    Eigen::Matrix4d m;
    m << v.x(), 0, 0, 0,
        0, v.y(), 0, 0,
        0, 0, v.z(), 0,
        0, 0, 0, 1;
    return m;
}
Mat4d Homogeneousd(const Mat3d& m)
{
    // Create a 4x4 matrix from the 3x3 matrix
    Eigen::Matrix4d Mat4d = Eigen::Matrix4d::Identity();
    Mat4d.block<3, 3>(0, 0) = m;
    return Mat4d;
}
Eigen::Matrix4d Translationd(const Vec3d& v)
{
    Eigen::Matrix4d m;
    m << 1, 0, 0, v.x(),
        0, 1, 0, v.y(),
        0, 0, 1, v.z(),
        0, 0, 0, 1;
    return m;
}
Mat4 Castd2f(const Mat4d& m)
{
    Mat4f result;
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            result(i, j) = m(i, j);
        }
    }
    return result;
}
Mat4d Castf2d(const Mat4& m)
{
    Mat4d result;
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            result(i, j) = m(i, j);
        }
    }
    return result;
}
std::tuple<Vec3, Quat, Vec3> DecomposeTransform(const Mat4& matrix)
{
    Vec3 T;
    Vec3 S;
    // 提取平移
    T = matrix.block<3, 1>(0, 3);
    // 提取旋转和缩放矩阵
    Eigen::Matrix3f R = matrix.block<3, 3>(0, 0);
    // 计算缩放
    S(0) = R.col(0).norm();
    S(1) = R.col(1).norm();
    S(2) = R.col(2).norm();
    // 去除缩放分量，得到纯旋转矩阵
    R.col(0) /= S(0);
    R.col(1) /= S(1);
    R.col(2) /= S(2);
    // 提取旋转四元数
    Eigen::Quaternionf Q(R);
    return std::make_tuple(T, Q, S);
}
}

} // namespace Aether::Math
