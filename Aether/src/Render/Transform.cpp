#include "Transform.h"
#include <Eigen/Geometry>
namespace Aether
{
    Eigen::Matrix4f Transform::Perspective(const float fovy, const float aspectRatio, const float zNear, const float zFar)
    {
        Eigen::Matrix4f projectionMatrix;
        projectionMatrix.setZero();

        const float tanHalfFovy = tanf(Eigen::AngleAxisf(fovy, Eigen::Vector3f::UnitY()).angle() / 2.0f);

        projectionMatrix(0, 0) = 1.0f / (aspectRatio * tanHalfFovy);
        projectionMatrix(1, 1) = 1.0f / tanHalfFovy;
        projectionMatrix(2, 2) = -(zFar + zNear) / (zFar - zNear);
        projectionMatrix(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
        projectionMatrix(3, 2) = -1.0f;

        return projectionMatrix;
    }

	Eigen::Matrix4f Transform::Translation(const Eigen::Vector3f& v)
	{
		Eigen::Matrix4f m;
		m << 1, 0, 0, v.x(),
			0, 1, 0, v.y(),
			0, 0, 1, v.z(),
			0, 0, 0, 1;
		return m;
	}

	Eigen::Matrix4f Transform::Scale(const Eigen::Vector3f& v)
	{
		Eigen::Matrix4f m;
		m << v.x(), 0, 0, 0,
			0, v.y(), 0, 0,
			0, 0, v.z(), 0,
			0, 0, 0, 1;
		return m;
	}
	Eigen::Matrix4f Transform::RotateX(float a)
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
	Eigen::Matrix4f Transform::RotateY(float a)
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
	Eigen::Matrix4f Transform::RotateZ(float a)
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
	Eigen::Matrix4f Transform::Rotation(const Eigen::Vector3f& v)
	{
		return RotateZ(v.z())*RotateY(v.y())*RotateX(v.x());
	}
	Eigen::Matrix4f Transform::Identity()
	{
		Eigen::Matrix4f m;
		m << 1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1;
		return m;
	}
}
