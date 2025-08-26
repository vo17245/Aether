#version 450 core


const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

out vec4 FragColor;
in vec3 v_Normal;
in vec3 v_FragPos;

// material parameters





#ifdef USE_AO_TEX
uniform sampler2D u_AoTex;
#else
uniform float u_Ao;
#endif

#ifdef USE_ROUGHNESS_TEX
uniform sampler2D u_RoughnessTex;
#else
uniform float u_Roughness;
#endif

#ifdef USE_ALBEDO_TEX
uniform sampler2D u_AlbedoTex;
#else
uniform vec3  u_Albedo;
#endif

#ifdef USE_METALLIC_TEX
uniform sampler2D u_MetallicTex;
#else
uniform float u_Metallic;
#endif



in vec2 v_TexCoords;


// lights
uniform vec3 u_LightPositions[16];
uniform vec3 u_LightColors[16];
uniform int u_LightCnt;

uniform vec3 u_CamPos;





void main()
{       
    #ifdef USE_ALBEDO_TEX
    vec3 albedo = texture(u_AlbedoTex, v_TexCoords).rgb;
    #else
    vec3 albedo = u_Albedo;
    #endif
    #ifdef USE_ROUGHNESS_TEX
    float roughness = texture(u_RoughnessTex, v_TexCoords).r;
    #else
    float roughness = u_Roughness;
    #endif
    #ifdef USE_METALLIC_TEX
    float metallic = texture(u_MetallicTex, v_TexCoords).r;
    #else
    float metallic = u_Metallic;
    #endif
    #ifdef USE_AO_TEX
    float ao = texture(u_AoTex, v_TexCoords).r;
    #else
    float ao = u_Ao;
    #endif

    vec3 N = normalize(v_Normal);
    vec3 V = normalize(u_CamPos-v_FragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    //vec3 debug_specluar;
    //vec3 debug_radiance;
    //float debug_distance;
    for(int i = 0; i < u_LightCnt; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(u_LightPositions[i] - v_FragPos);
        vec3 H = normalize(V + L);
        float distance    = length(u_LightPositions[i] - v_FragPos);
        //debug_distance=distance;
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = u_LightColors[i] * attenuation;        
        //debug_radiance=radiance;
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);       

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;     

        vec3 nominator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
        vec3 specular     = nominator / denominator;
        //debug_specluar=specular;
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
   
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    FragColor = vec4(color, 1.0);
}  