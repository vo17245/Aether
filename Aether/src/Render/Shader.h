#pragma once
#include "../Core/Core.h"
#include <string>
#include <Eigen/Core>
#include <unordered_map>
#include <optional>
#include "../Core/Config.h"
/*
* #aether_shader_command
* use_model
* use_view
* use_projection
* use_model_view
* use_model_view_projection
* use_normal_matrix
*/
namespace Aether
{
struct ShaderLoadResult;
class Shader
{
private:
	Shader() :m_RendererId(-1) {}
public:
	Shader(Shader&& shader) = delete;
	Shader(const Shader& shader) = delete;
	~Shader();
	void Bind()const;
	void Unbind()const;
	bool SetVec3f(const std::string& name, const Eigen::Vector3f& v);
	bool SetVec4f(const std::string& name, const Eigen::Vector4f& v);
	bool SetMat3f(const std::string& name, const Eigen::Matrix3f& m);
	bool SetMat4f(const std::string& name, const Eigen::Matrix4f& m);
	bool SetFloat(const std::string& name, const float n);
	bool SetInt(const std::string& name, const int n);
	bool GetLocation(const std::string& name, uint32_t& location);
	std::vector<std::string> GetCommands() { return m_AetherShaderCommands; }
public:
	static ShaderLoadResult CreateRefFromMem(const char* p, size_t len);
	static ShaderLoadResult CreateRefFromFile(const char* path);
private:
	uint32_t m_RendererId;
	std::unordered_map<std::string, uint32_t> m_LocationCache;
	std::vector<std::string> m_CompileErrors;
	std::vector<std::string> m_AetherShaderCommands;
	std::string m_Path;
public:
	class Premake
	{
	public:
		static Ref<Shader>& GetBasic();
	};
	
};
struct ShaderLoadResult
{
	std::optional<Ref<Shader>> shader;
	std::vector<std::string> errors;
	explicit operator bool()const noexcept { return bool(shader); }
	ShaderLoadResult(const ShaderLoadResult&) = default;
	ShaderLoadResult(ShaderLoadResult&&) = default;
	ShaderLoadResult() = default;
};

}//namespace Aether