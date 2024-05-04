
#version 450 core

layout(location = 0) in vec3 a_Position;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_FragPos;

bool near_line(vec2 pos)
{
    float a=mod(pos.x,1.0);
    float b=mod(pos.y,1.0);
    float delta=0.01;
    a=abs(a);
    b=abs(b);
    return ( a<delta || (1.0-a)<delta ) && ( b<delta || (1.0-b)<delta );
}
void main()
{
    v_FragPos=a_Position;
    vec4 pos=u_View*vec4(a_Position,0.0);
    pos.w=1.0;
    pos=u_Projection*pos;
    if(near_line(a_Position.xy))
    {
        gl_Position = pos.xyww;
    }
    else
    {
        gl_Position = pos;
    }
}

