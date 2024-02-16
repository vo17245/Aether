
#version 450 core

layout(location = 0) in vec3 a_Position;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_FragPos;

void main()
{
    v_FragPos=a_Position;
    vec4 pos=u_View*vec4(a_Position,0.0);
    pos.w=1.0;
    pos=u_Projection*pos;
    gl_Position = pos.xyww;
}

