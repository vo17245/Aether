#pragma once

#include <Eigen/Core>
#include "../Core/Core.h"

//用于创建mvp矩阵

AETHER_NAMESPACE_BEGIN
class Math
{
public:
	static constexpr inline const float PI = 3.1415926535f;
	static Eigen::Matrix4f Translation(float x, float y, float z);
	static Eigen::Matrix4f Scale(float x, float y, float z);
	static Eigen::Matrix4f RotateX(float a);//使用弧度
	static Eigen::Matrix4f RotateY(float a);
	static Eigen::Matrix4f RotateZ(float a);
	static Eigen::Matrix4f Identity();
	static Eigen::Matrix4f Press(float n, float f);
	static Eigen::Matrix4f Perspective(float n, float f, float t, float b, float r, float l);
	static Eigen::Matrix4f Orthographic(float n, float f, float t, float b, float r, float l);
};

AETHER_NAMESPACE_END