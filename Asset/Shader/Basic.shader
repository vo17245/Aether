
#aether_shader_command
use_model_view_projection
use_model_view
#vertex_shader
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location=1) in vec3 a_Normal;
uniform mat4 u_ModelViewProjection;
uniform mat4 u_ModelView;
out vec3 v_Normal;
out vec3 v_FragPos;
void main()
{
    v_Normal=(transpose(inverse(u_ModelView))*vec4(a_Normal,0)).xyz;
    v_FragPos=(u_ModelView*vec4(a_Position,1)).xyz;
    gl_Position = u_ModelViewProjection*vec4(a_Position,1.0);
}

#fragment_shader
#version 460 core

out vec4 color;
in vec3 v_Normal;
in vec3 v_FragPos;


uniform vec3 baseColor =vec3(.82 ,.67, .16);
uniform float metallic = 1;
uniform float subsurface = 0;
uniform float specular = .5;
uniform float roughness = .5;
uniform float specularTint = 0;
uniform float anisotropic = 0;
uniform float sheen = 0;
uniform float sheenTint = .5;
uniform float clearcoat= 0;
uniform float clearcoatGloss = 1;
const float PI = 3.14159265358979323846;


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
uniform DirectLight u_DirectLight[MAX_POINT_LIGHTS];
uniform int u_DirectLightCnt;
vec3 BRDF( vec3 L, vec3 V, vec3 N );
vec3 BSDF( vec3 baseColor, float roughness, float NoV, float NoL, float VoH );
void main()
{
    vec3 colorSum=vec3(0,0,0);
    for(int i=0;i<u_PointLightCnt;i++)
    {
        vec3 lightPos=u_PointLights[i].position;
        vec3 L=normalize(lightPos-v_FragPos);
        vec3 N=normalize(v_Normal);
        vec3 V=normalize(-v_FragPos);
        vec3 H=normalize(V+L);
        float NdotV=dot(N,V);
        float NdotL=dot(N,L);
        float NdotH=dot(N,H);
        float VdotH=dot(V,H);
        float diffuse=dot(N,L);
        float roughness=0.1;
        vec3 objColor=vec3(1,1,1);
        colorSum = BRDF(L,V,H)*BSDF(objColor,roughness,NdotV,NdotL,VdotH)*objColor;
    }
    color=vec4(colorSum,1);
}
float sqr(float x) { return x*x; }

float SchlickFresnel(float u)
{
    float m = clamp(1-u, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float GTR1(float NdotH, float a)
{
    if (a >= 1) return 1/PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (PI*log(a2)*t);
}

float GTR2(float NdotH, float a)
{
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

float smithG_GGX(float NdotV, float alphaG)
{
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}

vec3 mon2lin(vec3 x)
{
    return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}
vec3 BSDF( vec3 baseColor, float roughness, float NoV, float NoL, float VoH )
{
	float FD90 = 0.5 + 2 * VoH * VoH * roughness;
	float FdV = 1 + (FD90 - 1) * pow(( 1 - NoV ),5);
	float FdL = 1 + (FD90 - 1) * pow(( 1 - NoL ),5);
	return baseColor * ( (1 / PI) * FdV * FdL );
}
vec3 BRDF( vec3 L, vec3 V, vec3 N )
{
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    vec3 Cdlin = mon2lin(baseColor);
    float Cdlum = .3*Cdlin[0] + .6*Cdlin[1]  + .1*Cdlin[2]; // luminance approx.

    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1); // normalize lum. to isolate hue+sat
    vec3 Cspec0 = mix(specular*.08*mix(vec3(1), Ctint, specularTint), Cdlin, metallic);
    vec3 Csheen = mix(vec3(1), Ctint, sheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2 * LdotH*LdotH * roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    float Fss90 = LdotH*LdotH*roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

    // specular
    float aspect = sqrt(1-anisotropic*.9);
    float ax = max(.001, sqr(roughness)/aspect);
    float ay = max(.001, sqr(roughness)*aspect);
    float a=roughness*roughness;
    float Ds = GTR2(NdotH, a);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs  = smithG_GGX(NdotL, a);
    Gs *= smithG_GGX(NdotV, a);

    // sheen
    vec3 Fsheen = FH * sheen * Csheen;

    // clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(.1,.001,clearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

    return ((1/PI) * mix(Fd, ss, subsurface)*Cdlin + Fsheen)
        * (1-metallic)
        + Gs*Fs*Ds + .25*clearcoat*Gr*Fr*Dr;
}