#pragma once

#include "Eigen/Core"
#include "../Core/Core.h"
#include "../Core/Math.h"
namespace Aether
{
	class Camera
	{
	public:
		Camera() {}
		~Camera() {}
		inline Mat4& GetView() { return m_View; }
		inline Mat4& GetProjection() { return m_Projection; }
		inline const Mat4& GetView()const { return m_View; }
		inline const Mat4& GetProjection()const { return m_Projection; }
	protected:
		Mat4 m_View;
		Mat4 m_Projection;
	};
	class PerspectiveCamera :public Camera
	{
	public:
		PerspectiveCamera(float fovy, float zNear, float zFar, float aspectRatio)
			:m_Fovy(fovy), m_ZNear(zNear),
			m_ZFar(zFar), m_AspectRatio(aspectRatio),
			m_Position(0, 0, 0), m_Face(0, 0, -1)

		{
			CalculateProjection();
			CalculateView();
		}
	private:
		Real m_Fovy, m_ZNear, m_ZFar, m_AspectRatio;
		Vec3 m_Position;
		Vec3 m_Face;

	public:
		void CalculateProjection();
		void CalculateView();
		inline Mat4& GetView() { return m_View; }
		inline Mat4& GetProjection() { return m_Projection; }
		inline float& GetFovy() { return m_Fovy; }
		inline float& GetZNear() { return m_ZNear; }
		inline float& GetZFar() { return m_ZFar; }
		inline float& GetAspectRatio() { return m_AspectRatio; }
		inline Vec3& GetPosition() { return m_Position; }
		inline Vec3& GetFace() { return m_Face; }

		inline const Mat4& GetView()const  { return m_View; }
		inline const Mat4& GetProjection()const { return m_Projection; }
		inline const float& GetFovy()const { return m_Fovy; }
		inline const float& GetZNear()const { return m_ZNear; }
		inline const float& GetZFar()const { return m_ZFar; }
		inline const float& GetAspectRatio()const { return m_AspectRatio; }
		inline const Vec3& GetPosition()const { return m_Position; }
		inline const Vec3& GetFace()const { return m_Face; }

		inline void SetAspectRatio(Real aspectRatio) { m_AspectRatio = aspectRatio; }

	};
	class OrthographicCamera:public Camera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top, float near, float far);
		inline Vec3& GetPosition() { return m_Position; }
		inline Vec3& GetRotation() { return m_Rotation; }
		inline const Vec3& GetPosition()const { return m_Position; }
		inline const Vec3& GetRotation()const { return m_Rotation; }
		void CalculateView();
		void CalculateProjection();
		inline Real& GetLeft() { return m_Left; }
		inline Real& GetRight() { return m_Right; }
		inline Real& GetButtom() { return m_Bottom; }
		inline Real& GetTop() { return m_Top; }
		inline Real& GetNear() { return m_Near; }
		inline Real& GetFar() { return m_Far; }
		inline const Real& GetLeft()const { return m_Left; }
		inline const Real& GetRight()const { return m_Right; }
		inline const Real& GetButtom()const { return m_Bottom; }
		inline const Real& GetTop()const { return m_Top; }
		inline const Real& GetNear()const { return m_Near; }
		inline const Real& GetFar()const { return m_Far; }
	private:
		Real m_Left, m_Right, m_Bottom, m_Top, m_Near, m_Far;
		Vec3 m_Position;
		Vec3 m_Rotation;
	};
}


