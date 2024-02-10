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
		AETHER_DEBUG_LOG_ERROR("uniform {} not find", name);
		return false;
	}
	m_LocationCache[name] = location;
	return true;
}



Ref<Shader> Shader::Create(const ShaderSource& src)
{
	bool ret;
	uint32_t vsId;
	uint32_t fsId;
	std::string error;
	ret = CompileShader(GL_VERTEX_SHADER, src.GetVertexSource(), vsId, error);
	if (!ret)
	{
		return nullptr;
	}
	ret = CompileShader(GL_FRAGMENT_SHADER,src.GetFragmentSource(),fsId,error);
	if (!ret)
	{
		return nullptr;
	}
	auto rendererId = LinkShader(vsId, fsId);
	Ref<Shader> shader(new Shader);
	shader->m_RendererId = rendererId;
	return shader;
}

}//namespace Aether