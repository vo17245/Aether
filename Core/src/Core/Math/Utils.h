#pragma once
#include "Def.h"
#include <tuple>
namespace Aether {
namespace Math {
Eigen::Matrix4f Perspective(const float fovy, const float aspectRatio, const float zNear, const float zFar);
Eigen::Matrix4f Translation(const Vec3& v);
Eigen::Matrix4d Translationd(const Vec3d& v);
Eigen::Matrix4f Scale(const Vec3& v);
Eigen::Matrix4d Scaled(const Vec3d& v);
Eigen::Matrix4f RotateX(float a);
Eigen::Matrix4f RotateY(float a);
Eigen::Matrix4f RotateZ(float a);
Eigen::Matrix4f Rotation(const Vec3& v);
Eigen::Matrix4f Identity();
Eigen::Matrix4f Orthographic(float left, float right, float bottom, float top, float near, float far);
/**
 *@brief
 *  generate a rotation matrix that rotates around an input axis following the right-hand screw rule
 */
Mat4f Rotation(const Vec3f& axis, const float angle);
/**
if a and b are normalized,
then the return matrix is the rotation matrix that rotates a to b
*/
Mat3f GetRotation(const Vec3f& a, const Vec3f& b);
Mat4f Homogeneous(const Mat3f& m);
Mat4d Homogeneousd(const Mat3d& m);
Mat4 Castd2f(const Mat4d& m);
Mat4d Castf2d(const Mat4& m);
/**
 * @return tuple[T,Q,S] ([T,R,S])
 */
std::tuple<Vec3, Quat, Vec3> DecomposeTransform(const Mat4& matrix);
Mat3 MergeTranslation(const Mat2& m,const Vec2& v);
Mat4 MergeTranslation(const Mat3& m,const Vec3& v);
}
} // namespace Aether::Math
