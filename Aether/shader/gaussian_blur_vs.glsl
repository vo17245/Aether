#version 450 core

layout(location = 0) in vec3 a_Position;
out vec2 TexCoords;
void main()
{
    v_TexCoords=(a_Position.xy+1.0)/2.0;
    gl_Position = a_Position;
}

