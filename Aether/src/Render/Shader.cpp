#include "Shader.h"
#include <fstream>
#include <vector>
#include <sstream>
#include "OpenGLApi.h"
AETHER_NAMESPACE_BEGIN
static bool ReadShader(const std::string& path, std::string& vs, std::string& fs)
{
	std::ifstream fin;
	fin.open(path);
	if (!fin.is_open())
	{
		return false;
	}
	std::string line;
	//status
	//	0 ignore
	//  1 read vertex shader
	//  2 read fragment shader
	std::stringstream ss[2];
	int status = 0;
	while (std::getline(fin, line))
	{
		if (status == 0)
		{
			if (line.find("#vertex_shader") != std::string::npos)
			{
				status = 1;
				continue;
			}
		}
		else if (status == 1)
		{
			if (line.find("#fragment_shader") != std::string::npos)
			{
				status = 2;
				continue;
			}
			ss[0] << line<<"\r\n";
		}
		else if (status == 2)
		{
			ss[1] << line<<"\r\n";
		}
	}
	vs = std::move(ss[0].str());
	fs = std::move(ss[1].str());
	return true;
}
static bool CompileShader(uint32_t type, const std::string& src,uint32_t& id)
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
		std::cerr << "[OpenGL Error] " << typeStr << " Shader Compile Error \r\n" << message << std::endl;;
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
Shader::Shader(const std::string& path)
	:m_Path(path)
{
	std::string vs;
	std::string fs;
	ReadShader(path, vs, fs);
	uint32_t vsId;
	uint32_t fsId;
	CompileShader(GL_VERTEX_SHADER, vs, vsId);
	CompileShader(GL_FRAGMENT_SHADER, fs, fsId);
	m_RendererId = LinkShader(vsId, fsId);
	Bind();
}

Shader::~Shader()
{
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

void Shader::SetVec3f(const std::string& name, const Eigen::Vector3f& v)const
{
	uint32_t location;
	GetLocation(name, location);
	GLCall(glUniform3f(location, v.data()[0], v.data()[1], v.data()[2]));
}

void Shader::SetVec4f(const std::string& name, const Eigen::Vector4f& v)const
{
	uint32_t location;
	GetLocation(name, location);
	GLCall(glUniform4f(location, v.data()[0], v.data()[1], v.data()[2], v.data()[3]));
}

void Shader::SetMat3f(const std::string& name, const Eigen::Matrix3f& m)const
{
	uint32_t location;
	GetLocation(name, location);
	GLCall(glUniformMatrix3fv(location, 1, GL_FALSE, m.data()));
}

void Shader::SetMat4f(const std::string& name, const Eigen::Matrix4f& m)const
{
	uint32_t location;
	GetLocation(name, location);
	GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, m.data()));
}

void Shader::SetFloat(const std::string& name,const float n)const
{
	uint32_t location;
	GetLocation(name, location);
	GLCall(glUniform1f(location, n));
}

void Shader::SetInt(const std::string& name, const int n) const
{
	uint32_t location;
	GetLocation(name, location);
	GLCall(glUniform1i(location, n));
}

bool Shader::GetLocation(const std::string& name, uint32_t& location)const
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
		std::cerr << "uniform " << name << " not find in " << m_Path << std::endl;
		return false;
	}
	m_LocationCache[name] = location;
	return true;
}
AETHER_NAMESPACE_END