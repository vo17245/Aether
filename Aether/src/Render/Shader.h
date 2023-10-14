#pragma once
#include "../Core/Core.h"
#include <string>
#include <Eigen/Core>
#include <unordered_map>
AETHER_NAMESPACE_BEGIN
class Shader
{
public:
	Shader(const std::string& path);
	~Shader();
	void Bind()const;
	void Unbind()const;
	void SetVec3f(const std::string& name,const Eigen::Vector3f& v)const;
	void SetVec4f(const std::string& name, const Eigen::Vector4f& v)const;
	void SetMat3f(const std::string& name, const Eigen::Matrix3f& m)const;
	void SetMat4f(const std::string& name, const Eigen::Matrix4f& m)const;
	void SetFloat(const std::string& name, const float n)const;
	void SetInt(const std::string& name, const int n)const;
	bool GetLocation(const std::string& name,uint32_t& location)const;
private:
	uint32_t m_RendererId;
	std::string m_Path;
	mutable std::unordered_map<std::string, uint32_t> m_LocationCache;
};
AETHER_NAMESPACE_END