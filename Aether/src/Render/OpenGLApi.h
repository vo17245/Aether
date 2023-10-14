#pragma once
#include "../Core/Core.h"
#include <GL/glew.h>
#include <iostream>
AETHER_NAMESPACE_BEGIN
void GLClearError();
GLenum GLGetError();
void GLPrintError(GLenum err, const char* func, const char* file, int line);
#define GLCall(x) GLClearError();\
    x;\
    GLPrintError(GLGetError(),#x,__FILE__,__LINE__)

enum class GLType :uint32_t
{
    BYTE=0x1400,
    UNSIGNED_BYTE = 0x1401,
    SHORT=0x1402,
    UNSIGNED_SHORT = 0x1403,
    UNSIGNED_INT = 0x1405,
    FLOAT = 0x1406,
};
enum class GLDrawMode :uint32_t
{
    POINTS           =        GL_POINTS,
    LINE_STRIP       =        GL_LINE_STRIP,
    LINE_LOOP        =        GL_LINE_LOOP,
    LINES            =        GL_LINES,
    TRIANGLE_STRIP   =        GL_TRIANGLE_STRIP,
    TRIANGLE_FAN     =        GL_TRIANGLE_FAN,
    TRIANGLES        =        GL_TRIANGLES,
};
class VertexArray;
class IndexBuffer;

struct Texture2DConfig
{
    Texture2DConfig(uint32_t wrap_s, uint32_t wrap_t)
        :WRAP_S(wrap_s), WRAP_T(wrap_t), BORDER_COLOR{ 0,0,0,0 },
        MIN_FILTER(GL_NEAREST), MAG_FILTER(GL_LINEAR)
    {}
    Texture2DConfig()
        :WRAP_S(GL_MIRRORED_REPEAT), WRAP_T(GL_MIRRORED_REPEAT), BORDER_COLOR{ 0,0,0,0 },
        MIN_FILTER(GL_NEAREST), MAG_FILTER(GL_LINEAR)
    {}
    inline void SetBorderColor(float r, float g, float b, float a) 
    {
        BORDER_COLOR[0] = r;
        BORDER_COLOR[1] = g;
        BORDER_COLOR[2] = b;
        BORDER_COLOR[3] = a;
    }
    uint32_t WRAP_S;
    uint32_t WRAP_T;
    float BORDER_COLOR[4];
    uint32_t MIN_FILTER;
    uint32_t MAG_FILTER;
};

class OpenGLApi
{
private:
    static size_t s_ViewportWidth;
    static size_t s_ViewportHeight;
public:
    static void SetClearColor(float r, float g, float b, float a);
    static void SetViewport(int x, int y, int width, int height);
    static void EnableDepthTest();
    static void ClearDepthBuffer();
    static void ClearColorBuffer();
    static void DrawElements(const VertexArray& va,const IndexBuffer& ib,GLDrawMode mode=GLDrawMode::TRIANGLES);
    static void SetTexture2DConfig(const Texture2DConfig& config);
    static void ActivateTexture(size_t index);
    static size_t GetViewportWidth() { return s_ViewportWidth; }
    static size_t GetViewportHeight() { return s_ViewportHeight; }
public:
    static size_t TypeSize(GLType glType);
};
AETHER_NAMESPACE_END