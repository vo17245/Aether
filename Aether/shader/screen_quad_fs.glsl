#version 450 core
out vec4 color;
in vec2 v_TexCoords;
uniform sampler2D u_Scene;
int main()
{
    vec4 color = texture(u_Scene, v_TexCoords);
    //tonemapping
    #ifdef TONEMAPPING
    color = color / (color + vec3(1.0));
    #endif 
    //gamma
    color = pow(color, vec3(1.0/2.2)); 
}