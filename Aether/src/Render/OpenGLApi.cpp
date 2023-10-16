#include "OpenGLApi.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
AETHER_NAMESPACE_BEGIN
void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
GLenum GLGetError()
{
    GLenum err = glGetError();
    return err;
}
void GLPrintError(GLenum err, const char* func, const char* file, int line)
{
    if (err == GL_NO_ERROR)
        return;
    const char* err_str = (const char*)glewGetErrorString(err);
    std::cerr << "[OpenGL Error] " << (uint32_t)err << " " << err_str << " in " << func << " " << file << ":" << line << std::endl;;
    exit(-1);
}

void OpenGLApi::SetClearColor(float r, float g, float b, float a)
{
    GLCall(glClearColor(r,g,b,a));
}

void OpenGLApi::SetViewport(int x, int y, int width, int height)
{
    GLCall(glViewport(x,y,width,height));
}

void OpenGLApi::EnableDepthTest()
{
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_LESS));
}

void OpenGLApi::ClearDepthBuffer()
{
    GLCall(glClear(GL_DEPTH_BUFFER_BIT));
}

void OpenGLApi::ClearColorBuffer()
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT  ));
}

void OpenGLApi::DrawElements(const VertexArray& va,const IndexBuffer& ib,GLDrawMode mode)
{
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(static_cast<GLenum>(mode), static_cast<GLsizei>(ib.GetCount()), GL_UNSIGNED_INT, 0));
}



void OpenGLApi::SetTexture2DConfig(const Texture2DConfig& config)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.WRAP_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.WRAP_T);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, config.BORDER_COLOR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.MIN_FILTER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.MAG_FILTER);
}

void OpenGLApi::ActivateTexture(size_t index)
{
    GLCall(glActiveTexture(static_cast<GLenum>(GL_TEXTURE0+index)));
}

size_t OpenGLApi::TypeSize(GLType glType)
{
    switch (glType)
    {
    case GLType::UNSIGNED_BYTE:
        return sizeof(unsigned char);
    case GLType::UNSIGNED_SHORT:
        return sizeof(unsigned short);
    case GLType::UNSIGNED_INT: 
        return sizeof(unsigned int);
    case GLType::FLOAT:
        return sizeof(float);
    default:
        assert(false && "unknown GLType");
        return 0;
    }
}
AETHER_NAMESPACE_END