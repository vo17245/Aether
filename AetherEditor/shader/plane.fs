#version 450 core


out vec4 color;
in vec3 v_FragPos;

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
    if(near_line(v_FragPos.xy))
    {
        color=vec4(1.0,1.0,1.0,1.0);
    }
    else
    {
        color=vec4(0.0,0.0,0.0,0.0);
    }
}
