#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
namespace Aether {
template<typename T>
using Vec2=Eigen::Matrix<T, 2, 1>;
template<typename T>
using Vec3=Eigen::Matrix<T, 3, 1>;
template<typename T>
using Vec4=Eigen::Matrix<T, 4, 1>;
template<typename T>
using Mat2=Eigen::Matrix<T, 2, 2>;
template<typename T>
using Mat3=Eigen::Matrix<T, 3, 3>;
template<typename T>
using Mat4=Eigen::Matrix<T, 4, 4>;
template<typename T>
requires std::is_floating_point_v<T>
using Quat=Eigen::Quaternion<T>;
template<typename T>
using Mat2x3=Eigen::Matrix<T, 2, 3>;

// float32
using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;
using Mat2f = Mat2<float>;
using Mat3f = Mat3<float>;
using Mat4f = Mat4<float>;
using Mat2x3f = Mat2x3<float>;


// double
using Vec2d = Vec2<double>;
using Vec3d = Vec3<double>;
using Vec4d = Vec4<double>;
using Mat2d = Mat2<double>;
using Mat3d = Mat3<double>;
using Mat4d = Mat4<double>;

// int32 
using Vec2i = Vec2<int32_t>;
using Vec3i = Vec3<int32_t>;
using Vec4i = Vec4<int32_t>;
using Mat2i = Mat2<int32_t>;
using Mat3i = Mat3<int32_t>;
using Mat4i = Mat4<int32_t>;

// uint32 
using Vec2u = Vec2<uint32_t>;
using Vec3u = Vec3<uint32_t>;
using Vec4u = Vec4<uint32_t>;
using Mat2u = Mat2<uint32_t>;
using Mat3u = Mat3<uint32_t>;
using Mat4u = Mat4<uint32_t>;

// Quaterniond
using Quatd = Quat<double>;
using Quatf = Quat<float>;


namespace Math {
inline constexpr float PI = 3.14159265358979323846f;
}

} // namespace Aether

