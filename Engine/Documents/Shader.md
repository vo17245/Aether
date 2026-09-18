## descriptor

1. glsl shader编写

uniform buffer使用set 0
sampler 使用set 1
ssbo 使用set 2

三种descriptor 的binding都从0开始

如果没有uniform buffer 而有sampler 和ssbo，那么 sampler的set为0 ssbo的set为1 以此类推

example
```cpp
#version 450
layout(location=0) in vec2 v_TexCoord;
layout(location=0) out vec4 FragColor;
layout(set=1,binding=0) uniform sampler2D u_Texture;
layout(set=0,binding=0) uniform UniformBuffer
{
    float gamma;
}ubo;
void main()
{
vec4 textureColor=texture(u_Texture,v_TexCoord);
FragColor=vec4(pow(textureColor.rgb,vec3(ubo.gamma)),textureColor.a);
}
```


2. 相关代码

这个创建逻辑被写在 Aether::vk::DynamicDescriptorPool::CreateSet in Render\src\Render\Vulkan\DynamicDescriptorPool.h中
上层对render api的抽象DeviceDescriptorPool 最终调用Aether::vk::DynamicDescriptorPool::CreateSet