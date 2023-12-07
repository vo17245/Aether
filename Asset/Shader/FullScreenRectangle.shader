#aether_shader_command
use_model_view_projection
#vertex_shader
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

uniform mat4 u_ModelViewProjection;
out vec2 v_TexCoords;
void main()
{
    gl_Position = u_ModelViewProjection*vec4(a_Position,1.0);
    v_TexCoords=a_TexCoords;
}

#fragment_shader
#version 460 core

out vec4 color;
in vec2 v_TexCoords;
uniform sampler2D u_Texture;
void main()
{
    vec4 texColor=texture(u_Texture, v_TexCoords);
    texColor.x=pow(texColor.x,2.2);
    texColor.y=pow(texColor.y,2.2);
    texColor.z=pow(texColor.z,2.2);
    texColor.w=1;
    color = texColor;
    
}