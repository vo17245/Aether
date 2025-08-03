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
    //perspectiveMatrix(1, 1) = -1.0f / tanHalfFovy;
    perspectiveMatrix(1, 1) = 1.0f / tanHalfFovy;
    perspectiveMatrix(2, 2) = -(zFar + zNear) / (zFar - zNear);
    perspectiveMatrix(2, 3) = -2.0f * zFar * zNear / (zFar - zNear);
    perspectiveMatrix(3, 2) = -1.0f;

    return perspectiveMatrix;
}

Eigen::Matrix4f Translation(const Vec3f& v)
{
    Eigen::Matrix4f m;
    m << 1, 0, 0, v.x(),
        0, 1, 0, v.y(),
        0, 0, 1, v.z(),
        0, 0, 0, 1;
    return m;
}

Eigen::Matrix4f Scale(const Vec3f& v)
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
Eigen::Matrix4f Rotation(const Vec3f& v)
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

std::tuple<Vec3f, Quat, Vec3f> DecomposeTransform(const Mat4f& matrix)
{
    Vec3f T;
    Vec3f S;
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
Mat3f MergeTranslation(const Mat2f& m,const Vec2f& v)
{
    Mat3f result = Mat3f::Identity();
    result.block<2, 2>(0, 0) = m;
    result.block<2, 1>(0, 2) = v;
    return result;
}
Mat4f MergeTranslation(const Mat3f& m,const Vec3f& v)
{
    Mat4f result = Mat4f::Identity();
    result.block<3, 3>(0, 0) = m;
    result.block<3, 1>(0, 3) = v;
    return result;
}
Mat4f LookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up)
{
    // 计算相机坐标系的三个轴
    Vec3f forward = (target - eye).normalized();  // 前方向（Z轴负方向）
    Vec3f right = forward.cross(up).normalized(); // 右方向（X轴正方向）
    Vec3f upNew = right.cross(forward);           // 重新计算上方向（Y轴正方向）
    
    // 构建视图矩阵
    // 注意：这里forward取负号，因为在右手坐标系中相机看向Z轴负方向
    Mat4f viewMatrix;
    viewMatrix << 
        right.x(),    upNew.x(),   -forward.x(),  0,
        right.y(),    upNew.y(),   -forward.y(),  0,
        right.z(),    upNew.z(),   -forward.z(),  0,
        -right.dot(eye), -upNew.dot(eye), forward.dot(eye), 1;
    
    return viewMatrix;
}
Mat2x3f CreateAffine2D(float sx,float sy,float tx,float ty,float r)
{
    Mat2x3f m;
    m(0,2)=tx;
    m(1,2)=ty;
    float cosR = std::cos(r);
    float sinR = std::sin(r);
    m(0,0) = cosR * sx;
    m(0,1) = -sinR * sy;
    m(1,0) = sinR * sx;
    m(1,1) = cosR * sy;
    return m;
}
}

} // namespace Aether::Math
