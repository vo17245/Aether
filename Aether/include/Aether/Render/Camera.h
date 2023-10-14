#pragma once

#include "Eigen/Core"
#include "../Core/Core.h"
AETHER_NAMESPACE_BEGIN
class Camera
{
public:
	Camera():m_AspectRadio(1.0f) {}

	~Camera(){}
	Camera& operator=(const Camera& ori)
	{
		m_View = ori.m_View;
		m_Projection = ori.m_Projection;
		return *this;
	}
	inline const Eigen::Matrix4f& GetView()const { return m_View; }
	inline const Eigen::Matrix4f& GetProjection()const { return m_Projection; }
	inline const Eigen::Matrix4f& GetCameraMatrix()const { return m_CameraMatrix; }
	void CalculateCameraMatrix() ;
	void CalculateViewMatrix();
	void Rotate(Eigen::Matrix4f mat);
	void Translate(float x, float y, float z);
	void Scale(float x, float y, float z);
	inline void SetAspectRadio(float aspectRadio) { m_AspectRadio = aspectRadio; }
	static Camera CreatePerspectiveCamera(float aspectRadio);
private:
	Eigen::Matrix4f m_View;
	Eigen::Matrix4f m_Projection;
	float m_AspectRadio;
	Eigen::Matrix4f m_CameraMatrix;
private:
	Eigen::Matrix4f m_Rotation;
	Eigen::Matrix4f m_Scaling;
	Eigen::Matrix4f m_Translation;
};
AETHER_NAMESPACE_END