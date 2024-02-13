#pragma once
#include "Eigen/Core"
#include <Eigen/Dense>
namespace Aether
{
	using Real = float;//Real must be float32
	using Mat4 = Eigen::Matrix4f;
	using Vec3 = Eigen::Vector3f;
	using Vec4 = Eigen::Vector4f;
	using Vec2 = Eigen::Vector2f;

	using Vec3i = Eigen::Vector3i;
	using Vec4i = Eigen::Vector4i;
	using Vec2i = Eigen::Vector2i;
	namespace Math
	{
		constexpr Real PI=3.1415926535;
	}
}



