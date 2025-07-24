#include "Raster.h"
#include "QuadArrayMesh.h"
#include "Render/RenderApi/DeviceBuffer.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceMesh.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Quad.h"
#include "PackGlyph.h"
#include "Render/Utils.h"
#include <Debug/Log.h>
namespace Aether::Text
{
bool Raster::Render(RenderPassParam& param, RenderPassResource& resource)
{
    bool res;
    // update mesh  and uniform
    res = UpdateMesh(param, resource);
    if (!res)
    {
        return false;
    }
    res = UpdateUniformBuffer(param, resource);
    if (!res)
    {
        return false;
    }
    res = CreateDescriptorSet(param.descriptorPool);
    if (!res)
    {
        return false;
    }
    res = UpdateDescriptorSet(resource,param);
    if (!res)
    {
        return false;
    }
    res = RecordCommand(param, resource);
    if (!res)
    {
        return false;
    }

    return true;
}
bool Raster::Init(DeviceRenderPassView renderPass, bool enableBlend, DeviceDescriptorPool& descriptorPool,bool enableDepthTest)
{
    if (!CreateDescriptorSet(descriptorPool))
    {
        assert(false && "create descriptor set failed");
        return false;
    }
    if (!CreateShader())
    {
        assert(false && "create shader failed");
        return false;
    }
    if (!CreatePipeline(renderPass, enableBlend,enableDepthTest))
    {
        assert(false && "create pipeline failed");
        return false;
    }
    m_StaggingBuffer=DeviceBuffer::CreateForStaging(sizeof(HostUniformBuffer));
    m_CurveTextureSampler=DeviceSampler::CreateDefault();
    m_GlyphTextureSampler=DeviceSampler::CreateNearest();
    return true;
}
bool Raster::CreateShader()
{
    static const char* frag = R"(
#version 450
layout(location=0)flat in uint v_GlyphIndex;
layout(location=1)in vec2 v_UV;
layout(location=0)out vec4 FragColor;
layout(set=1,binding=0)uniform usampler2D u_GlyphTexture;
layout(set=1,binding=1)uniform sampler2D u_CurveTexture;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_MVP;
    vec4 u_Color; // RGB ,A is not used,just for alignment
}ubo;
struct Glyph {
	uint start, count;
};

struct Curve {
	vec2 p0, p1, p2;
};
uint g_GlyphInLine;
uint g_CurveInLine;
float g_AntiAliasingWindowSize = 2.0;
#define HINTING_SCALE 256

Glyph FetchGlyph(uint index)
{
    uint line=index/g_GlyphInLine;
    uint offset=(index%g_GlyphInLine)*2;
    uvec4 glyphData1=texelFetch(u_GlyphTexture,ivec2(offset,line),0);
    uvec4 glyphData2=texelFetch(u_GlyphTexture,ivec2(offset+1,line),0);
    Glyph glyph;
    glyph.start=glyphData1.x+(glyphData1.y<<8)+(glyphData1.z<<16)+(glyphData1.w<<24);
    glyph.count=glyphData2.x+(glyphData2.y<<8)+(glyphData2.z<<16)+(glyphData2.w<<24);
    return glyph;
}
Curve FetchCurve(uint index)
{
    uint line=index/g_CurveInLine;
    uint offset=(index%g_CurveInLine)*2;
    vec4 curveData1=texelFetch(u_CurveTexture,ivec2(offset,line),0);
    vec4 curveData2=texelFetch(u_CurveTexture,ivec2(offset+1,line),0);
    Curve curve;
    curve.p0.x=curveData1.x;
    curve.p0.y=curveData1.y;
    curve.p1.x=curveData1.z;
    curve.p1.y=curveData1.w;
    curve.p2.x=curveData2.x;
    curve.p2.y=curveData2.y;
    return curve;
}

/**
 * @brief set global variable g_GlyphInLine and g_CurveInLine
*/
void Init()
{
    ivec2 glyphTextureSize=textureSize(u_GlyphTexture,0);
    g_GlyphInLine=glyphTextureSize.x/2;
    ivec2 curveTextureSize=textureSize(u_CurveTexture,0);
    g_CurveInLine=uint(float(curveTextureSize.x)/2);
}
float CalculateCoverage(float inverseDiameter, vec2 p0, vec2 p1, vec2 p2) {

	if (p0.y > 0 && p1.y > 0 && p2.y > 0) return 0.0;
	if (p0.y < 0 && p1.y < 0 && p2.y < 0) return 0.0;
	// Note: Simplified from abc formula by extracting a factor of (-2) from b.
	vec2 a = p0 - 2*p1 + p2;
	vec2 b = p0 - p1;
	vec2 c = p0;

	float t0, t1;
	if (abs(a.y) >= 1e-5) {
		// Quadratic segment, solve abc formula to find roots.
		float radicand = b.y*b.y - a.y*c.y;
		if (radicand <= 0) return 0.0;
	
		float s = sqrt(radicand);
		t0 = (b.y - s) / a.y;
		t1 = (b.y + s) / a.y;
	} else {
		// Linear segment, avoid division by a.y, which is near zero.
		// There is only one root, so we have to decide which variable to
		// assign it to based on the direction of the segment, to ensure that
		// the ray always exits the shape at t0 and enters at t1. For a
		// quadratic segment this works 'automatically', see readme.
        if(abs(p0.y - p2.y)<1e-5)
        {
            return 0.0;
        }
		float t = p0.y / (p0.y - p2.y);
		if (p0.y < p2.y) {
			t0 = -1.0;
			t1 = t;
		} else {
			t0 = t;
			t1 = -1.0;
		}
	}

	float alpha = 0;
	if (t0 >= 0 && t0 < 1) {
		float x = (a.x*t0 - 2.0*b.x)*t0 + c.x;
		alpha += clamp(x * inverseDiameter + 0.5, 0, 1);
	}

	if (t1 >= 0 && t1 < 1) {
		float x = (a.x*t1 - 2.0*b.x)*t1 + c.x;
		alpha -= clamp(x * inverseDiameter + 0.5, 0, 1);
	}
    
	return alpha;
}
vec2 Hinting(vec2 p,float scale)
{
    p=p*scale;
    p.x=floor(p.x);
    p.y=floor(p.y);
    p=p/scale;
    return p;
}
vec2 HintingCenter(vec2 p,float scale)
{
    p=p*scale;
    p.x=floor(p.x)+0.5;
    p.y=floor(p.y)+0.5;
    p=p/scale;
    return p;
}   
// 简单哈希函数
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}
// 2D噪声函数
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(hash(i + vec2(0.0, 0.0)), 
                   hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), 
                   hash(i + vec2(1.0, 1.0)), u.x), u.y);
}
// 分形噪声
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}
vec3 rainbowNoise(vec2 uv) {
    // 使用fwidth来计算自适应缩放
    vec2 uvDerivative = fwidth(uv);
    float adaptiveScale = 1.0 / max(uvDerivative.x, uvDerivative.y);
    
    // 调整基础缩放以获得理想的噪声密度
    float baseScale = 0.1; // 可调整的基础密度
    float finalScale = adaptiveScale * baseScale;
    
    float t = fbm(uv * finalScale);
    
    // 创建彩虹色谱
    vec3 color1 = vec3(1.0, 0.0, 0.0); // 红
    vec3 color2 = vec3(1.0, 0.5, 0.0); // 橙
    vec3 color3 = vec3(1.0, 1.0, 0.0); // 黄
    vec3 color4 = vec3(0.0, 1.0, 0.0); // 绿
    vec3 color5 = vec3(0.0, 0.0, 1.0); // 蓝
    vec3 color6 = vec3(0.5, 0.0, 1.0); // 紫
    
    if (t < 0.2) return mix(color1, color2, t * 5.0);
    else if (t < 0.4) return mix(color2, color3, (t - 0.2) * 5.0);
    else if (t < 0.6) return mix(color3, color4, (t - 0.4) * 5.0);
    else if (t < 0.8) return mix(color4, color5, (t - 0.6) * 5.0);
    else return mix(color5, color6, (t - 0.8) * 5.0);
}
float CalculateCoverageAtUv(vec2 uv,vec2 inverseDiameter)
{
    float alpha = 0.0;
    Glyph glyph = FetchGlyph(v_GlyphIndex);
    for (int i = 0; i < glyph.count; i++) {
		Curve curve = FetchCurve(glyph.start+i);
        curve.p0=Hinting(curve.p0,inverseDiameter.x*HINTING_SCALE);
        curve.p1=Hinting(curve.p1,inverseDiameter.x*HINTING_SCALE);
        curve.p2=Hinting(curve.p2,inverseDiameter.x*HINTING_SCALE);
		vec2 p0 = curve.p0 - uv;
		vec2 p1 = curve.p1 - uv;
		vec2 p2 = curve.p2 - uv;


		alpha += CalculateCoverage(inverseDiameter.x, p0, p1, p2);

        // 旋转90度
        p0 = vec2(-p0.y, p0.x);
        p1 = vec2(-p1.y, p1.x);
        p2 = vec2(-p2.y, p2.x);
        alpha += CalculateCoverage(inverseDiameter.x, p0, p1, p2);

	}
    alpha/=2;
    return alpha;
}
float CalculateCoverageAtUv_Oversampling(vec2 uv,vec2 inverseDiameter)
{
    vec2 delta=fwidth(uv)*0.2;
    float alpha = 0.0;
    for(int i=-2;i<=2;++i)
    {
        for(int j=-2;j<=2;++j)
        {
            vec2 offset=vec2(i,j)*delta;
            alpha += CalculateCoverageAtUv(uv+offset,inverseDiameter*5);
        }
    }
    return alpha/25;
}
void Fill()
{
    vec2 uv = v_UV;
    vec2 inverseDiameter = 1.0 / (g_AntiAliasingWindowSize * fwidth(v_UV));// uv到屏幕空间的缩放
    uv=HintingCenter(uv,inverseDiameter.x*HINTING_SCALE);
    #ifdef OVERSAMPLING
    float alpha=CalculateCoverageAtUv_Oversampling(uv,inverseDiameter);
    #else
    float alpha=CalculateCoverageAtUv(uv,inverseDiameter);
    #endif
    
    vec3 fontColor= ubo.u_Color.rgb;
    #ifdef COLORFUL_EDGE
    vec3 edgeColor=rainbowNoise(uv);
    vec3 color = mix(edgeColor, fontColor, clamp(alpha * 1.5, 0.0, 1.0));
    FragColor=vec4(color,alpha);
    #else
    FragColor = vec4(fontColor, alpha);
    #endif
}
void Sdf()
{
}
void main()
{
    Init();
    #ifdef FILL
    Fill();
    #endif
    #ifdef SDF
    Sdf();
    #endif
    
}
)";
    static const char* vert = R"(
#version 450
layout(location=0)in vec3 a_Position;
layout(location=1)in uint a_GlyphIndex;
layout(location=2)in vec2 a_UV;
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_MVP;
}ubo;

layout(location=0)flat out uint v_GlyphIndex;
layout(location=1)out vec2 v_UV;
void main()
{
    vec4 pos=vec4(a_Position,1.0);
    v_GlyphIndex=a_GlyphIndex;
    gl_Position=ubo.u_MVP*pos;
    v_UV=a_UV;

}
)";
    std::stringstream ss;
    if(IsFlagSet(m_Keywords,Keyword::ColorfulEdge))
    {
        ss<< "#define COLORFUL_EDGE\n";
    }
    if(IsFlagSet(m_Keywords,Keyword::Oversampling))
    {
        ss<< "#define OVERSAMPLING\n";
    }
    if(IsFlagSet(m_Keywords,Keyword::Fill))
    {
        ss<< "#define FILL\n";
    }
    if(IsFlagSet(m_Keywords,Keyword::Sdf))
    {
        ss<< "#define SDF\n";
    }
    auto shaderEx = DeviceShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vert),
                                         ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, 
                                            frag,ss.str()));
    if (!shaderEx)
    {
        Debug::Log::Error("create shader failed:\n{}",shaderEx.error());
        assert(false && "create shader failed");
        return false;
    }
    m_Shader = std::move(shaderEx.value());
    return true;
}
bool Raster::CreatePipeline(DeviceRenderPassView renderPass, bool enableBlend,bool enableDepthTest)
{
    vk::PipelineLayout::Builder layoutBuilder;
    layoutBuilder.AddDescriptorSetLayouts(m_DescriptorSet.GetVk().layouts);
    auto layoutOpt = layoutBuilder.Build();
    if (!layoutOpt)
    {
        assert(false && "failed to create pipeline layout");
        return false;
    }
    auto& layout = layoutOpt.value();
    vk::GraphicsPipeline::Builder builder(renderPass.GetVk(), layout);
    if(enableDepthTest)
    {
        builder.EnableDepthTest();
    }
    QuadArrayMesh dummyMesh;
    auto vertexBufferLayouts = dummyMesh.GetMesh().CreateVertexBufferLayouts();
    builder.PushVertexBufferLayouts(vertexBufferLayouts)
        .AddVertexStage(m_Shader.GetVk().vertex, "main")
        .AddFragmentStage(m_Shader.GetVk().fragment, "main");
    if (enableBlend)
    {
        builder.EnableBlend();
    }
    auto pipelineOpt = builder.Build();
    if (!pipelineOpt)
    {
        assert(false && "failed to create pipeline");
        return false;
    }
    auto& pipeline = pipelineOpt.value();

    m_Pipeline = std::move(pipeline);
    m_PipelineLayout = std::move(layout);
    return true;
}


bool Raster::CreateDescriptorSet(DeviceDescriptorPool& descriptorPool)
{
    auto set = descriptorPool.CreateSet(1, 0, 2);
    m_DescriptorSet = std::move(set);
    return true;
}
static Quad CreateQuad(const Font::Glyph& glyph, const Vec2f& pos, uint32_t glyphIndex)
{
    Quad res;
    res.glyphIndex = glyphIndex;
    res.position = pos;
    res.size.x() = glyph.width;
    res.size.y() = glyph.height;
    return res;
}
bool Raster::UpdateMesh(RenderPassParam& param, RenderPassResource& resource)
{
    // create host mesh data
    QuadArrayMesh mesh;
    bool res;
    float emSize = param.font.emSize;
    float worldSize = param.worldSize;
    float scale=worldSize/param.font.emSize;
    assert(param.unicodes.size()==param.glyphPosition.size());
    for(size_t i=0;i<param.unicodes.size();++i)
    {
        auto unicode=param.unicodes[i];
        auto& pos=param.glyphPosition[i];
        auto& glyph = param.font.glyphs[unicode];
        auto& bufferGlyph=param.font.bufferGlyphs[glyph.bufferIndex];
        if(bufferGlyph.count==0)
        {
            //skip empty glyph(like space)
            continue;
        }
       
        FT_Pos d = (FT_Pos) (emSize * m_Dilation);

		float u0 = (float)(glyph.bearingX-d) / emSize;
		float v0 = (float)(glyph.bearingY-glyph.height-d) / emSize;
		float u1 = (float)(glyph.bearingX+glyph.width+d) / emSize;
		float v1 = (float)(glyph.bearingY+d) / emSize;
		
        float x0=pos.x();
        float y0 = pos.y(); 
		float x1 = x0 + glyph.width*scale;
        float y1 = y0 + glyph.height*scale;
        // 创建quad
        Quad quad;
        quad.position = Vec2f(x0, y0);
        quad.size = Vec2f(x1 - x0, y1 - y0);
        quad.uv0 = Vec2f(u0, v0);
        quad.uv1 = Vec2f(u1, v1);
        quad.glyphIndex = glyph.bufferIndex;
        quad.z=param.z;
        mesh.PushQuad(quad);
       
    }
    // create device mesh data
    if (resource.mesh)
    {
        resource.mesh.Update(mesh.GetMesh());
    }
    else
    {
        resource.mesh = DeviceMesh::Create(mesh.GetMesh());
    }
    return true;
}
bool Raster::UpdateUniformBuffer(RenderPassParam& param, RenderPassResource& resource)
{
    // host
    auto& camera=param.camera;
    //========
    for (size_t i = 0; i < 16; i++)
    {
        m_HostUniformBuffer.mvp[i] = camera.matrix.data()[i];
    }
    m_HostUniformBuffer.color[0] = param.color.x();
    m_HostUniformBuffer.color[1] = param.color.y();
    m_HostUniformBuffer.color[2] = param.color.z();
    // stagging
    m_StaggingBuffer.SetData(std::span<uint8_t>((uint8_t*)&m_HostUniformBuffer,
                                                sizeof(m_HostUniformBuffer)));
    // uniform
    return DeviceBuffer::SyncCopy(resource.uniformBuffer, m_StaggingBuffer,
                                  sizeof(m_HostUniformBuffer), 0, 0);
}

bool Raster::UpdateDescriptorSet(RenderPassResource& resource,RenderPassParam& param)
{
    auto& descriptorSet = m_DescriptorSet.GetVk();
    
    // ubo
    {
        auto& uboAccessor = descriptorSet.ubos[0];
        auto& set = descriptorSet.sets[uboAccessor.set];
        vk::DescriptorSetOperator op(set);
        op.BindUBO(uboAccessor.binding, resource.uniformBuffer.GetVk());
        op.Apply();
    }
    //texture
    {
        auto& set = descriptorSet.sets[1];
        vk::DescriptorSetOperator op(set);
        op.BindSampler(0, m_GlyphTextureSampler.GetVk(), param.font.glyphTexture.GetDefaultImageView().GetVk());
        op.BindSampler(1, m_CurveTextureSampler.GetVk(), param.font.curveTexture.GetDefaultImageView().GetVk());
        op.Apply();
    }
    return true;
}
bool Raster::RecordCommand(RenderPassParam& param, RenderPassResource& resource)
{
    auto& commandBuffer = param.commandBuffer.GetVk();

    commandBuffer.BindPipeline(m_Pipeline.GetVk());
    auto& descriptorSet = m_DescriptorSet.GetVk();
    for (size_t i = 0; i < descriptorSet.sets.size(); ++i)
    {
        auto& set = descriptorSet.sets[i];
        commandBuffer.BindDescriptorSet(set, m_PipelineLayout.GetVk(), i);
    }
    Render::Utils::DrawMesh(commandBuffer, resource.mesh.GetVk());
    return true;
}
} // namespace Aether::Text