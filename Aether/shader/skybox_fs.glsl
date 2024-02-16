#version 450 core

uniform samplerCube u_EnvMap;
const float PI = 3.14159265359;

out vec4 color;
in vec3 v_FragPos;
void main()
{
    vec3 texColor=texture(u_EnvMap,v_FragPos).xyz;
    texColor = texColor / (texColor + vec3(1.0));
    texColor = pow(texColor, vec3(1.0/2.2));  
    color=vec4(texColor,1);
}
