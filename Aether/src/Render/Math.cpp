#include "Math.h"
#include <cmath>
AETHER_NAMESPACE_BEGIN
Eigen::Matrix4f Math::Translation(float x, float y, float z)
{
	Eigen::Matrix4f m;
	m << 1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1;
	return m;
}

Eigen::Matrix4f Math::Translation(Eigen::Vector3f v)
{
	return Translation(v.x(),v.y(),v.z());
}

Eigen::Matrix4f Math::Scale(float x, float y, float z)
{
	Eigen::Matrix4f m;
	m << x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1;
	return m;
}

Eigen::Matrix4f Math::Scale(Eigen::Vector3f v)
{
	return Scale(v.x(), v.y(), v.z());
}

Eigen::Matrix4f Math::RotateX(float a)
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

Eigen::Matrix4f Math::RotateY(float a)
{
	float sina = std::sin(a);
	float cosa = std::cos(a);
	Eigen::Matrix4f m;
	m << cosa, 0, sina, 0,
		0, 1, 0 , 0,
		-1*sina, 0, cosa, 0,
		0, 0, 0, 1;
	return m;
}

Eigen::Matrix4f Math::RotateZ(float a)
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
Eigen::Matrix4f Math::Identity()
{
	Eigen::Matrix4f m;
	m << 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1;
	return m;
}

Eigen::Matrix4f Math::Press(float n, float f)
{
	Eigen::Matrix4f m;
	m << 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, (n + f) / n,  -f,
		0, 0, 1 / n, 0;
	return m;
}

Eigen::Matrix4f Math::Perspective(float n, float f, float t, float b, float r, float l)
{
	auto m = Math::Press(n, f);
	m = Math::Orthographic(n, f, n*t/f, n * b/f, n*r/f, n*l/f)*m;
	return m;
}



Eigen::Matrix4f Math::Orthographic(float n, float f, float t, float b, float r, float l)
{
	auto m = Math::Translation(-1 * (l + r) / 2, -1 * (b + t) / 2, -1 * (n + f) / 2);//移到原点
	m = Math::Scale(2 / (r - l), 2/(t-b), 2 / (n - f)) * m;//缩放
	m = Math::RotateY(PI)*m;
	return m;
}
AETHER_NAMESPACE_END