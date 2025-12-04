#include "Sprite/Renderer.h"
namespace Aether::Sprite
{
bool CreatePipeline()
{
    static const char vert[]=R"(
    #version 450
layout(location=0)in vec2 a_Position;//屏幕坐标系,glyph的aabb
layout(location=1)in vec2 a_UV;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_MVP;// 只有view和projection矩阵,model矩阵是identity
}ubo;

layout(location=0)out vec2 v_UV;
void main()
{
    vec4 pos=vec4(a_Position,1.0);
    v_GlyphIndex=a_GlyphIndex;
    gl_Position=ubo.u_MVP*pos;
    v_UV=a_UV;

}
    )";
    return true;
}
void Renderer::DrawInstance(const InstanceDraw& instance, DeviceCommandBuffer& commandBuffer)
{
}

} // namespace Aether::Sprite