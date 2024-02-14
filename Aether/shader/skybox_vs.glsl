
#version 450 core

layout(location = 0) in vec3 a_Position;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_FragPos;

void main()
{
    v_FragPos=a_Position;
    gl_Position = u_Projection*vec4((u_View*vec4(a_Position,0.0)).xyz,1.0);
}

