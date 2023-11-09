#aether_shader_comment
do basic vertex projection ,an fill each pixel with color(0,0,0,1) 
#aether_shader_command
use_model_view_projection
#vertex_shader
#version 460 core

layout(location = 0) in vec3 a_Position;
uniform mat4 u_ModelViewProjection;
void main()
{
    gl_Position = u_ModelViewProjection*vec4(a_Position,1.0);
}

#fragment_shader
#version 460 core

out vec4 color;

void main()
{
    vec3 objColor=vec3(0.0,0.0,0.0);
    color = vec4(objColor,1.0);
}