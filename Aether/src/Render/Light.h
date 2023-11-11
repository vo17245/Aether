#pragma once
#include <Eigen/Core>
namespace Aether
{
	class Light
	{
	public:
		enum class LightType:int32_t
		{
			Direct,
			Point,
		};
		Light(LightType type,const Eigen::Vector3f& color)
			:m_Type(type),m_Color(color), HasShadow(false) {}
		bool HasShadow;
	protected:
		Eigen::Vector3f m_Color;
		LightType m_Type;
	};
	class DirectLight:public Light
	{
	public:
		DirectLight(const Eigen::Vector3f& color,
			const Eigen::Vector3f& direct)
			:Light(LightType::Direct, color),m_Direct(direct) {}
	private:
		Eigen::Vector3f m_Direct;
	};
	
}