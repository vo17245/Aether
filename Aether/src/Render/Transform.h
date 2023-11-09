#pragma once
#include <Eigen/Core>
namespace Aether
{
	class Transform
	{
	public:
		static Eigen::Matrix4f Perspective(const float fovy, const float aspectRatio, const float zNear, const float zFar);
		static constexpr inline const float PI = 3.1415926535f;
		static Eigen::Matrix4f Translation(const Eigen::Vector3f& v);
		static Eigen::Matrix4f Scale(const Eigen::Vector3f& v);
		static Eigen::Matrix4f RotateX(float a);//使用弧度
		static Eigen::Matrix4f RotateY(float a);
		static Eigen::Matrix4f RotateZ(float a);
		static Eigen::Matrix4f Rotation(const Eigen::Vector3f& v);
		static Eigen::Matrix4f Identity();
	};
}

