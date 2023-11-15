#include "Shader.h"
#include <fstream>
#include <vector>
#include <sstream>
#include "OpenGLApi.h"
#include "../Core/Log.h"
#include <filesystem>
#include "../Core/Assert.h"
namespace Aether
{
static std::string & trim(std::string & s)
{
	if (s.empty())
	{
		return s;
	}
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

static bool ParseShader(std::istream& is, std::string& vs, std::string& fs,
	std::vector<std::string>& aetherShaderCommands)
{

	std::string line;
	//status
	//	0 ignore
	//  1 read vertex shader
	//  2 read fragment shader
	//  3 read aether shader commands
	//	4 read comment,comment will be ignored
	std::stringstream ss[2];
	int status = 0;
	while (std::getline(is, line))
	{
		if (status == 0)
		{
			if (line.find("#vertex_shader") != std::string::npos)
			{
				status = 1;
				continue;
			}
			if (line.find("#fragment_shader") != std::string::npos)
			{
				status = 2;
				continue;
			}
			if (line.find("#aether_shader_command") != std::string::npos)
			{
				status = 3;
				continue;
			}
			if (line.find("#aether_shader_comment") != std::string::npos)
			{
				status = 4;
				continue;
			}
			
		}
		else if (status == 1)
		{
			if (line.find("#vertex_shader") != std::string::npos)
			{
				status = 1;
				continue;
			}
			if (line.find("#fragment_shader") != std::string::npos)
			{
				status = 2;
				continue;
			}
			if (line.find("#aether_shader_command") != std::string::npos)
			{
				status = 3;
				continue;
			}
			if (line.find("#aether_shader_comment") != std::string::npos)
			{
				status = 4;
				continue;
			}
			
			ss[0] << line << "\r\n";
		}
		else if (status == 2)
		{
			if (line.find("#vertex_shader") != std::string::npos)
			{
				status = 1;
				continue;
			}
			if (line.find("#fragment_shader") != std::string::npos)
			{
				status = 2;
				continue;
			}
			if (line.find("#aether_shader_command") != std::string::npos)
			{
				status = 3;
				continue;
			}
			if (line.find("#aether_shader_comment") != std::string::npos)
			{
				status = 4;
				continue;
			}
			
			ss[1] << line << "\r\n";
		}
		else if (status == 3)
		{
			if (line.find("#vertex_shader") != std::string::npos)
			{
				status = 1;
				continue;
			}
			if (line.find("#fragment_shader") != std::string::npos)
			{
				status = 2;
				continue;
			}
			if (line.find("#aether_shader_command") != std::string::npos)
			{
				status = 3;
				continue;
			}
			if (line.find("#aether_shader_comment") != std::string::npos)
			{
				status = 4;
				continue;
			}
			if (line.size() == 0)
			{
				continue;
			}
			aetherShaderCommands.push_back(trim(line));
		}
		else if (status == 4)
		{
			if (line.find("#vertex_shader") != std::string::npos)
			{
				status = 1;
				continue;
			}
			if (line.find("#fragment_shader") != std::string::npos)
			{
				status = 2;
				continue;
			}
			if (line.find("#aether_shader_command") != std::string::npos)
			{
				status = 3;
				continue;
			}
			if (line.find("#aether_shader_comment") != std::string::npos)
			{
				status = 4;
				continue;
			}
		}
	}
	vs = std::move(ss[0].str());
	fs = std::move(ss[1].str());
	return true;
}

static bool CompileShader(uint32_t type, const std::string& src,uint32_t& id,std::string& compileError)
{
	GLCall( id=glCreateShader(type));
	const char* src_data = src.c_str();
	GLCall(glShaderSource(id, 1, &src_data, nullptr));
	GLCall(glCompileShader(id));
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length];

		glGetShaderInfoLog(id, length, &length, message);
		const char* typeStr = type == GL_VERTEX_SHADER ? "Vertex" : "Fragment";
		std::stringstream compileErrorStream;
		compileErrorStream<<"[OpenGL Error] "<< typeStr<<" Shader Compile Error \r\n"
			<< message << std::endl;;
		compileError = compileErrorStream.str();
		
		AETHER_DEBUG_LOG_ERROR("[OpenGL Error] {} Shader Compile Error \r\n {}",
			typeStr, message);
		
		delete[] message;
		return false;
	}
	return true;
}
static uint32_t LinkShader(uint32_t vs, uint32_t fs)
{
	uint32_t program_id = glCreateProgram();
	GLCall(glAttachShader(program_id, vs));
	GLCall(glAttachShader(program_id, fs));
	GLCall(glLinkProgram(program_id));
	GLCall(glValidateProgram(program_id));
	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));
	return program_id;
}

Shader::~Shader()
{
	if (m_RendererId == -1)
		return;
	GLCall(glDeleteProgram(m_RendererId));
}

void Shader::Bind()const
{
	GLCall(glUseProgram(m_RendererId));
}

void Shader::Unbind()const
{
	GLCall(glUseProgram(0));
}

bool Shader::SetVec3f(const std::string& name, const Eigen::Vector3f& v)
{
	uint32_t location;
	if (!GetLocation(name, location))
	{
		return false;
	}
	GLCall(glUniform3f(location, v.data()[0], v.data()[1], v.data()[2]));
	return true;
}

bool Shader::SetVec4f(const std::string& name, const Eigen::Vector4f& v)
{
	uint32_t location;
	if (!GetLocation(name, location))
	{
		return false;
	}
	GLCall(glUniform4f(location, v.data()[0], v.data()[1], v.data()[2], v.data()[3]));
	return true;
}

bool Shader::SetMat3f(const std::string& name, const Eigen::Matrix3f& m)
{
	uint32_t location;
	if (!GetLocation(name, location))
	{
		return false;
	}
	GLCall(glUniformMatrix3fv(location, 1, GL_FALSE, m.data()));
	return true;
}

bool Shader::SetMat4f(const std::string& name, const Eigen::Matrix4f& m)
{
	uint32_t location;
	if (!GetLocation(name, location))
	{
		return false;
	}
	GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, m.data()));
	return true;
}

bool Shader::SetFloat(const std::string& name,const float n)
{
	uint32_t location;
	if (!GetLocation(name, location))
	{
		return false;
	}
	GLCall(glUniform1f(location, n));
	return true;
}

bool Shader::SetInt(const std::string& name, const int n) 
{
	uint32_t location;
	if (!GetLocation(name, location))
	{
		return false;
	}
	GLCall(glUniform1i(location, n));
	return true;
}

bool Shader::GetLocation(const std::string& name, uint32_t& location)
{
	auto iter = m_LocationCache.find(name);
	if (iter != m_LocationCache.end())
	{
		location=iter->second;
		return true;
	}
	GLCall(location = glGetUniformLocation(m_RendererId, name.c_str()));
	if (location == -1)
	{
		AETHER_DEBUG_LOG_ERROR("uniform {} not find in {}", name, m_Path);
		return false;
	}
	m_LocationCache[name] = location;
	return true;
}
ShaderLoadResult Shader::CreateRefFromMem(const char* p, size_t len)
{
	std::string text(p,p+len);
	std::istringstream iss(text);
	ShaderLoadResult res;
	Ref<Shader> shader(new Shader());
	std::string vs;
	std::string fs;
	std::vector<std::string> aetherShaderCommands;
	std::vector<std::string> errors;
	ParseShader(iss, vs, fs, aetherShaderCommands);
	uint32_t vsId;
	uint32_t fsId;
	std::string error;
	bool ret;
	ret=CompileShader(GL_VERTEX_SHADER, vs, vsId,error);
	if (!ret)
	{
		res.errors.push_back(error);
		return res;
	}
		
	ret=CompileShader(GL_FRAGMENT_SHADER, fs, fsId,error);
	if (!ret)
	{
		res.errors.push_back(error);
		return res;
	}
	auto rendererId= LinkShader(vsId, fsId);
	shader->m_RendererId = rendererId;
	shader->Bind();
	res.shader = shader;

	return res;
}
ShaderLoadResult Shader::CreateRefFromFile(const char* path)
{
	ShaderLoadResult res;
	std::string vs;
	std::string fs;
	std::ifstream ifs(path);
	if (!ifs.is_open())
	{
		AETHER_DEBUG_LOG_ERROR("failed to open file {}", path);
		res.errors.emplace_back("failed to open file");
		return res;
	}
	std::vector<std::string> aetherShaderCommands;
	std::vector<std::string> errors;
	ParseShader(ifs, vs, fs, aetherShaderCommands);
	uint32_t vsId;
	uint32_t fsId;
	std::string error;
	bool ret;
	ret=CompileShader(GL_VERTEX_SHADER, vs, vsId,error);
	if (!ret)
	{
		res.errors.emplace_back(std::move(error));
		return res;
	}
		
	ret=CompileShader(GL_FRAGMENT_SHADER, fs, fsId, error);
	if (!ret)
	{
		res.errors.emplace_back(std::move(error));
		return res;
	}
		
	auto rendererId= LinkShader(vsId, fsId);
	Ref<Shader> shader(new Shader());
	shader->m_Path = path;
	shader->m_RendererId = rendererId;
	shader->Bind();
	shader->m_AetherShaderCommands = aetherShaderCommands;
	res.shader = shader;
	return res;
}
Ref<Shader>& Shader::Premake::GetBasic()
{
	static ShaderLoadResult shader =
		(
			Shader::CreateRefFromFile
			(
				(
					std::filesystem::path(GetConfig().resource_path) / "Shader/Premake/Basic.shader"
				).string().c_str()
			)
		);
	AETHER_ASSERT(shader && "Failed to init premake basic shader");
	return shader.shader.value();
}
}//namespace Aether