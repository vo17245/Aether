#pragma once
#include <Eigen/Core>
#include "../Core/Math.h"
namespace Aether
{
	class Light
	{
	public:
		Light(const Vec3& color)
			:HasShadow(false),m_Color(color)  {}
		Light(Light&&)=default;
		Light(const Light&)=default;
		bool HasShadow;
		inline Vec3& GetColor() { return m_Color; }
		inline const Vec3& GetColor() const { return m_Color; }
	protected:
		Vec3 m_Color;
		
	};
	class DirectLight:public Light
	{
	public:
		DirectLight(const DirectLight&) = default;
		DirectLight(const Vec3& color,
			const Vec3& direct)
			:Light( color),m_Direct(direct) {}
		inline Vec3& GetDirect() { return m_Direct; }
		inline const Vec3& GetDirect() const { return m_Direct; }
	private:
		Vec3 m_Direct;
	};
	class PointLight :public Light
	{
	public:
		PointLight(const PointLight&) = default;
		PointLight(PointLight&&)=default;
		PointLight(const Vec3& color,const Vec3& pos)
			:Light(color), m_Position(pos) {}
		inline Vec3& GetPosition() { return m_Position; }
		inline const Vec3& GetPosition() const { return m_Position; }
	private:
		Vec3 m_Position;
	};
	
}