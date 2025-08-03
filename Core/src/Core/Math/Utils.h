#pragma once
#include "Def.h"
#include <tuple>
namespace Aether {
namespace Math {
Eigen::Matrix4f Perspective(const float fovy, const float aspectRatio, const float zNear, const float zFar);
Eigen::Matrix4f Translation(const Vec3f& v);
Eigen::Matrix4d Translationd(const Vec3d& v);
Eigen::Matrix4f Scale(const Vec3f& v);
Eigen::Matrix4d Scaled(const Vec3d& v);
Eigen::Matrix4f RotateX(float a);
Eigen::Matrix4f RotateY(float a);
Eigen::Matrix4f RotateZ(float a);
Eigen::Matrix4f Rotation(const Vec3f& v);
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
/**
 * @return tuple[T,Q,S] ([T,R,S])
 */
std::tuple<Vec3f, Quat, Vec3f> DecomposeTransform(const Mat4f& matrix);
Mat3f MergeTranslation(const Mat2f& m,const Vec2f& v);
Mat4f MergeTranslation(const Mat3f& m,const Vec3f& v);
Mat4f LookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up);
Mat2x3f CreateAffine2D(float sx,float sy,float tx,float ty,float r);
}
} // namespace Aether::Math
