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

// light
#define MAX_DIRECT_LIGHTS 10
#define MAX_POINT_LIGHTS 10
struct DirectLight
{
    vec3 direction;
    vec3 color;
};
struct PositionLight
{
    vec3 position;
    vec3 color;
};
uniform PointLight u_PointLights[MAX_DIRECT_LIGHTS];  
uniform int u_PointLightCnt;
uniform DirectLight u_DirectLight[MAX_POINT_LIGHTS];
uniform int u_DirectLightCnt;

//varying

in vec3 v_FragPos;
in vec3 v_Normal;
vec3 OnDirectLight(DirectLight light)
{
    return vec3(0,0,1);
}
vec3 OnPointLight(PointLight light)
{
    return vec3(0,0,1);
}
void main()
{
    vec3 sumColor=vec3(0,0,0);
    for(int  i=0;i<MAX_DIRECT_LIGHTS;i++)
    {
        //direct light
        sumColor=sumColor+OnDirectLight(u_PointLights[i]);
    }
    for(int  i=0;i<MAX_POINT_LIGHTS;i++)
    {
        //point light
        sumColor=sumColor+OnPointLight(u_DirectLight[i]);
    }
    gl_FragColor=vec4(color,1);
    
}