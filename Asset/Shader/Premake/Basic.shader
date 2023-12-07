#aether_shader_command
use_model_view_projection
use_model_view
use_normal_matrix
#vertex_shader
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location=1) in vec3 a_Normal;
uniform mat4 u_ModelViewProjection;
uniform mat4 u_ModelView;
uniform mat4 u_NormalMatrix;
out vec3 v_Normal;
out vec3 v_FragPos;
void main()
{
    v_Normal=(u_NormalMatrix*vec4(a_Normal,0)).xyz;
    v_FragPos=(u_ModelView*vec4(a_Position,1)).xyz;
    gl_Position = u_ModelViewProjection*vec4(a_Position,1.0);
}

#fragment_shader
#version 460 core
out vec4 color;
// light
#define MAX_DIRECT_LIGHTS 10
#define MAX_POINT_LIGHTS 10
struct DirectLight
{
    vec3 direction;
    vec3 color;
};
struct PointLight
{
    vec3 position;
    vec3 color;
};
uniform PointLight u_PointLights[MAX_DIRECT_LIGHTS];  
uniform int u_PointLightCnt;
uniform DirectLight u_DirectLights[MAX_POINT_LIGHTS];
uniform int u_DirectLightCnt;

//varying

in vec3 v_FragPos;
in vec3 v_Normal;

void main()
{
    vec3 N=normalize(v_Normal);
    vec3 V=normalize(-v_FragPos);
    vec3 sumColor=vec3(0,0,0);
    vec3 baseColor=vec3(0.8,0.6,0.4);
    for(int  i=0;i<u_DirectLightCnt;i++)
    {
        vec3 L=normalize(u_DirectLights[i].direction);
        sumColor+=dot(L,N)*baseColor*u_DirectLights[i].color;
    }
    for(int  i=0;i<u_PointLightCnt;i++)
    {
        //point light
        vec3 L=normalize(u_PointLights[i].position-v_FragPos);
        sumColor+=dot(L,N)*baseColor*u_PointLights[i].color;
       
    }
    color=vec4(pow(sumColor.x,2.2),pow(sumColor.y,2.2),pow(sumColor.z,2.2),1);
    
}