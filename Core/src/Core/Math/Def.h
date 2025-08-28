#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <format>
namespace Aether {
// float32
using Vec2f = Eigen::Vector2f;
using Vec3f = Eigen::Vector3f;
using Vec4f = Eigen::Vector4f;
using Mat2f = Eigen::Matrix2f;
using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;
using Mat2x3f = Eigen::Matrix<float, 2, 3>;


// double
using Vec2d = Eigen::Vector2d;
using Vec3d = Eigen::Vector3d;
using Vec4d = Eigen::Vector4d;
using Mat2d = Eigen::Matrix2d;
using Mat3d = Eigen::Matrix3d;
using Mat4d = Eigen::Matrix4d;

// int32 
using Vec2i = Eigen::Vector2<int32_t>;
using Vec3i = Eigen::Vector3<int32_t>;
using Vec4i = Eigen::Vector4<int32_t>;
using Mat2i = Eigen::Matrix2<int32_t>;
using Mat3i = Eigen::Matrix3<int32_t>;
using Mat4i = Eigen::Matrix4<int32_t>;

// uint32 
using Vec2u = Eigen::Vector2<uint32_t>;
using Vec3u = Eigen::Vector3<uint32_t>;
using Vec4u = Eigen::Vector4<uint32_t>;
using Mat2u = Eigen::Matrix2<uint32_t>;
using Mat3u = Eigen::Matrix3<uint32_t>;
using Mat4u = Eigen::Matrix4<uint32_t>;

// Quaterniond
using Quatd = Eigen::Quaterniond;
using Quat = Eigen::Quaternionf;



namespace Math {
inline constexpr float PI = 3.14159265358979323846f;
}

} // namespace Aether

