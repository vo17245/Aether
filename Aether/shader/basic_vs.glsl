
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location=1) in vec3 a_Normal;
uniform mat4 u_ModelViewProjection;
uniform mat4 u_ModelView;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_NormalMatrix;

out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{
    v_Normal=(u_NormalMatrix*vec4(a_Normal,0)).xyz;
    v_FragPos=(u_ModelView*vec4(a_Position,1)).xyz;
    gl_Position = u_ModelViewProjection*vec4(a_Position,1.0);
}

