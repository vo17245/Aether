#include "CubeMap.h"
#include "Aether/Core/Assert.h"
#include "Aether/Core/Log.h"
namespace Aether
{
    static int GetGLFormat(const Image& image)
    {
        auto format=image.GetFormat();
        switch(format)
        {
            case ImageFormat::RGB888:
                return GL_RGB;
            case ImageFormat::RGBA8888:
                return GL_RGBA;
            case ImageFormat::RGB_FLOAT32:
                return GL_RGB16F;
            case ImageFormat::RGBA_FLOAT32:
                return GL_RGBA16F;
            default:
                AETHER_ASSERT(false&&"Unkown Image Format");
                AETHER_DEBUG_LOG_ERROR("Unkown Image Format");
                Log::Warn("Unkown Image Format");
                return GL_RGB;
        }
    }
    static int GetGLDataType(const Image& image)
    {
        auto format=image.GetFormat();
        switch(format)
        {
            case ImageFormat::RGB888:
                return GL_UNSIGNED_BYTE;
            case ImageFormat::RGBA8888:
                return GL_UNSIGNED_BYTE;
            case ImageFormat::RGB_FLOAT32:
                return GL_FLOAT;
            case ImageFormat::RGBA_FLOAT32:
                return GL_FLOAT;
            default:
                AETHER_ASSERT(false&&"Unkown Image Format");
                AETHER_DEBUG_LOG_ERROR("Unkown Image Format");
                Log::Warn("Unkown Image Format");
                return GL_UNSIGNED_BYTE;
        }
    }
    static int SetData(int target,const Image& image)
    {
        int gl_format=GetGLFormat(image);
        int gl_data_type=GetGLDataType(image);
        GLCall(glTexImage2D(target , 
                     0, gl_format,
                      image.GetWidth(), 
                     image.GetHeight(), 0, gl_format,gl_data_type, image.GetData()
        ));
    }
    CubeMap::CubeMap(const Image& px,const Image& nx,const Image& py,const Image&ny,const Image& pz,const Image& nz)
    {
        GLCall(glGenTextures(1, &m_RendererID));
        GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
        

        

        SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, px);
        SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, nx);

        SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, py);
        SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, ny);

        SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, pz);
        SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, nz);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,         GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,         GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,         GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    CubeMap::~CubeMap()
    {
        glDeleteTextures(1, &m_RendererID);
    }
    void CubeMap::SetPositiveX(const Image& image)
    {
        SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_X, image);
    }
    void CubeMap::SetNegativeX(const Image& image)
    {
        SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, image);
    }
    void CubeMap::SetPositiveY(const Image& image)
    {
        SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, image);
    }
    void CubeMap::SetNegativeY(const Image& image)
    {
        SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, image);
    }
    void CubeMap::SetPositiveZ(const Image& image)
    {
        SetData(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, image);
    }
    void CubeMap::SetNegativeZ(const Image& image)
    {
        SetData(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, image);
    }
    CubeMap::CubeMap()
    {
        GLCall(glGenTextures(1, &m_RendererID));
        GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,         GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,         GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,         GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
}