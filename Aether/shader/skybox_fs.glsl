#version 450 core

uniform samplerCube u_EnvMap;
const float PI = 3.14159265359;

out vec4 color;
in vec3 v_FragPos;
void main()
{
    vec4 texColor=texture(u_EnvMap,v_FragPos);
    texColor.xyz = pow(texColor.xyz, vec3(1.0/2.2));  
    color=vec4(texColor.xyz,1);
}
