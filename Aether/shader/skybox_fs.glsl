#version 450 core

uniform sampler2D u_EnvMap;
const float PI = 3.14159265359;
float AngleBetween(vec3 a, vec3 b) {
    float AdotB=dot(a,b);
    float cosAB=AdotB/(length(a)*length(b));
    return acos(cosAB);
}
vec2 FragPos2UV(vec3 pos)
{

    vec2 uv; // 二维纹理坐标

    float r = length(pos);
    float u = (atan(pos.y,pos.x)+PI) / (  2*PI);
    float v = (acos(pos.z)+PI/2) / (PI) ;

    uv = vec2(u, v);
    return uv;
}

out vec4 color;
in vec3 v_FragPos;
void main()
{
    vec4 texColor=texture(u_EnvMap,FragPos2UV(v_FragPos));
    color=vec4(texColor.xyz,1);
}
