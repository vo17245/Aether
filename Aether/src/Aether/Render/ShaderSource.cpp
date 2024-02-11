#include "ShaderSource.h"
#include "Aether/Core/Assert.h"
#include "Aether/Core/Log.h"
#include "Aether/Render/ShaderSource.h"
#include "Aether/Utils/FileUtils.h"
#include <filesystem>
#include <optional>
#include <sstream>
#include "Aether/Core/Config.h"
namespace Aether
{
    void ShaderSource::AddLineInVertexShader(const std::string& line)
    {
        //1.find the end of fitst line
        size_t pos=0;
        while(pos<line.size())
        {
            if(m_VertexSource[pos]=='\n')
            {
                break;
            }
        } 
        if(pos==m_VertexSource.size())
        {
            //error handle
            m_VertexSource+=line+"\n";
            return;
        }
        //2.insert new line after first line
        std::stringstream ss;
        ss<<m_VertexSource.substr(0,pos+1);
        ss<<line<<"\n";
        if(pos==m_VertexSource.size()-1)
        {

        }
        else 
        {
            ss<<m_VertexSource.substr(pos+1,m_VertexSource.size());
        }
        m_VertexSource=ss.str();
    }
    void ShaderSource::AddLineInFragmentShader(const std::string& line)
    {
        //1.find the end of fitst line
        size_t pos=0;
        while(pos<line.size())
        {
            if(m_FragmentSource[pos]=='\n')
            {
                break;
            }
        } 
        if(pos==m_FragmentSource.size())
        {
            //error handle
            m_FragmentSource+=line+"\n";
            return;
        }
        //2.insert new line after first line
        std::stringstream ss;
        ss<<m_FragmentSource.substr(0,pos+1);
        ss<<line<<"\n";
        if(pos==m_FragmentSource.size()-1)
        {

        }
        else 
        {
            ss<<m_FragmentSource.substr(pos+1,m_FragmentSource.size());
        }
        m_FragmentSource=ss.str();
    }
    void ShaderSource::AddVertexShaderMacro(const std::string& macro)
    {
        std::stringstream ss;
        ss<<"#define "<<macro<<"\n";
        AddLineInVertexShader(ss.str());
        
    }
    void ShaderSource::AddFragmentShaderMacro(const std::string& macro)
    {
        std::stringstream ss;
        ss<<"#define "<<macro<<"\n";
        AddLineInFragmentShader(ss.str());
    }

    std::optional<Ref<ShaderSource>> ShaderSource::LoadFromFile(
        const std::filesystem::path& vsPath,
    const std::filesystem::path& fsPath)
    {
        
        auto opt_vs=FileUtils::ReadFileAsString(vsPath);
        if(!opt_vs)
        {
            return std::nullopt;
        }
        auto opt_fs=FileUtils::ReadFileAsString(fsPath);
        if(!opt_fs)
        {
            return std::nullopt;
        }
        Ref<ShaderSource> res(new ShaderSource);
        res->m_FragmentSource=std::move(opt_fs.value());
        res->m_VertexSource=std::move(opt_vs.value());
        return res;
    }
    static std::filesystem::path GetBuiltinVertexShaderPath(BuiltinShaderType type)
    {
        switch (type) 
        {
        case BuiltinShaderType::BASIC:
        {
            std::stringstream ss;
            ss<<GetConfig().shader_dir<<"/"<<"basic_vs.glsl";
            return ss.str();
        }
        break;
        case BuiltinShaderType::PBR:
        {
            std::stringstream ss;
            ss<<GetConfig().shader_dir<<"/"<<"pbr_vs.glsl";
            return ss.str();
        }
        break;
        default:
        AETHER_ASSERT(false&&"unknown builtin shader");
        AETHER_DEBUG_LOG_ERROR("unknown builtin shader");
        return "";
        break;
        }
    }
    static std::filesystem::path GetBuiltinFragmentShaderPath(BuiltinShaderType type)
    {
        switch (type) 
        {
            case BuiltinShaderType::BASIC:
            {
                std::stringstream ss;
                ss<<GetConfig().shader_dir<<"/"<<"basic_fs.glsl";
                return ss.str();
            }
            break;
            case BuiltinShaderType::PBR:
            {
                std::stringstream ss;
                ss<<GetConfig().shader_dir<<"/"<<"pbr_fs.glsl";
                return ss.str();
            }
            break;
            default:
            AETHER_ASSERT(false&&"unknown builtin shader");
            AETHER_DEBUG_LOG_ERROR("unknown builtin shader");
            return "";
            break;
        }
    }
    std::optional<Ref<ShaderSource>> ShaderSource::LoadBuiltin(BuiltinShaderType type)
    {
        return LoadFromFile(GetBuiltinVertexShaderPath(type), 
        GetBuiltinFragmentShaderPath(type));
    }
}