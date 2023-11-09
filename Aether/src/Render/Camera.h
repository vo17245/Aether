#pragma once

#include "Eigen/Core"
#include "../Core/Core.h"
AETHER_NAMESPACE_BEGIN

class Camera
{
public:
	Camera() {}
	~Camera(){}
	inline Eigen::Matrix4f& GetView() { return m_View; }
	inline Eigen::Matrix4f& GetProjection() { return m_Projection; }
	inline const Eigen::Matrix4f& GetView()const { return m_View; }
	inline const Eigen::Matrix4f& GetProjection()const  { return m_Projection; }
protected:
	Eigen::Matrix4f m_View;
	Eigen::Matrix4f m_Projection;
};
class PerspectiveCamera:public Camera
{
public:
	PerspectiveCamera(float fovy, float zNear, float zFar, float aspectRatio)
		:m_Fovy(fovy), m_ZNear(zNear),
		m_ZFar(zFar), m_AspectRatio(aspectRatio) {}
private:
	float m_Fovy, m_ZNear, m_ZFar, m_AspectRatio;
	Eigen::Vector3f m_Position;
	Eigen::Vector3f m_Rotation;
	
public:
	void CalculateProjection();
	void CalculateView();
	inline Eigen::Matrix4f& GetView() { return m_View; }
	inline Eigen::Matrix4f& GetProjection() { return m_Projection; }
	inline float& GetFovy() { return m_Fovy; }
	inline float& GetZNear() { return m_ZNear; }
	inline float& GetZFar() { return m_ZFar; }
	inline float& GetAspectRatio() { return m_AspectRatio; }
};
AETHER_NAMESPACE_END