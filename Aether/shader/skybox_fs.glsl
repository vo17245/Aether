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
    float a=AngleBetween(vec3(pos.x,0,pos.z),vec3(1,0,0));
    if(pos.z>0)
    {
        a+=PI;
    }
    vec3 t=normalize(pos);
    float b=t.y;
    return vec2((a)/(2*PI),b/2);
}

out vec4 color;
in vec3 v_FragPos;
void main()
{
    color=vec4(FragPos2UV(v_FragPos),0,1);
}
