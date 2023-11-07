#vertex_shader
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location=1) in vec3 a_Normal;
uniform mat4 u_MVP;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Normal;
out vec3 v_FragmentPosition;
void main()
{
    gl_Position = u_MVP*vec4(a_Position,1.0);
    v_Normal=(transpose(inverse(u_View*u_Model)) * vec4(a_Normal,0)).xyz;
    v_FragmentPosition=(u_View*u_Model*vec4(a_Position,1.0)).xyz;
}

#fragment_shader
#version 460 core

out vec4 color;
in vec3 v_Normal;
uniform vec3 u_LightColor;

in vec3 v_FragmentPosition;
const float PI=3.1415926535;
//N 宏观法向量 H 半程向量 normlize(lightDir+viewDir)
float GGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a;
    float denom = (NdotH2 * (a - 1.0) + 1.0);
    denom = 3.14159 * denom * denom+0.00001;

    return nom / denom;
}
float Schlick(float F0,float NdotV)
{

    float F = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
    return F;
}
float CalculateSpecular(vec3 V,vec3 L,vec3 N,float roughness,float F0)
{
    N=normalize(N);
    V=normalize(V);
    L=normalize(L);
    float NdotV=dot(N,V);
    float D=GGX(N,  normalize(L+V),  roughness);
    float F=Schlick(F0,NdotV);
    return F*D; 
}

void main()
{
    float F0 = 0.04; // 金属的基础反射率
    vec3 lightPos=vec3(10,10,10);
    vec3 lightDir=normalize(lightPos-v_FragmentPosition);
    vec3 viewDir=v_FragmentPosition;
    float roughness=0.2;
    vec3 objColor=vec3(1,0,0);
    float ambient=0.2;
    
    float specular=CalculateSpecular(viewDir,lightDir,v_Normal,roughness,F0);
    color = vec4(vec3(specular*objColor),1);
}