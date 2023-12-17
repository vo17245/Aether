#pragma once
#include <Eigen/Core>
#include "../Core/Math.h"
#include "Aether/Core/Math.h"
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
namespace Aether
{
	class Transform
	{
	public:
		static Eigen::Matrix4f Perspective(const float fovy, const float aspectRatio, const float zNear, const float zFar);
		static constexpr inline const float PI = 3.1415926535f;
		static Eigen::Matrix4f Translation(const Eigen::Vector3f& v);
		static Eigen::Matrix4f Scale(const Eigen::Vector3f& v);
		static Eigen::Matrix4f RotateX(float a);
		static Eigen::Matrix4f RotateY(float a);
		static Eigen::Matrix4f RotateZ(float a);
		static Eigen::Matrix4f Rotation(const Eigen::Vector3f& v);
		static Eigen::Matrix4f Identity();
		static Eigen::Matrix4f Orthographic(float left, float right, float bottom, float top, float near, float far);
		/**
		 *@brief
		 *  generate a rotation matrix that rotates around an input axis following the right-hand screw rule
		*/
		static Mat4 Rotation(const Vec3 axis,const Real angle);
			
	};
}

