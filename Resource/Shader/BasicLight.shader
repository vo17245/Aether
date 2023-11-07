#vertex_shader
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location=1) in vec3 a_Normal;
uniform mat4 u_MVP;
uniform mat4 u_Model;
out vec3 v_Normal;
out vec3 v_FragmentPosition;
void main()
{
    gl_Position = u_MVP*vec4(a_Position,1.0);
    v_Normal=mat3(transpose(inverse(u_Model))) * a_Normal;
    v_FragmentPosition=a_Position;
}

#fragment_shader
#version 460 core

out vec4 color;
in vec3 v_Normal;
uniform vec3 u_LightColor;
uniform vec3 u_LightDirection;
in vec3 v_FragmentPosition;

void main()
{
    vec3 objColor=vec3(0.5,0.5,0.5);
    float ambient=0.2;
    float diffuse=max(0,dot(normalize(-u_LightDirection),normalize(v_Normal)));    
    color = vec4(objColor*u_LightColor*(ambient+diffuse),1);
}