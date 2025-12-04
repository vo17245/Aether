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
#define INF (1.0/0.0)
#define EPSILON 1e-10
#define PI 3.14159265358979323846
vec2 QuadraticBezier(vec2 P0, vec2 P1, vec2 P2, float t)
{
    float u = 1.0 - t;
    return u * u * P0 + 2.0 * u * t * P1 + t * t * P2;
}
/**
 * @brief 计算点到贝塞尔曲线的最近距离,如果最近的点不在贝塞尔上(t不在[0-1]),返回到最近端点的距离
 * @param p0 p1 p2 端点 控制点 端点
*/
float NearestToBezier(vec2 P0,vec2 P1,vec2 P2,vec2 M)
{
    vec2 A=P1-P0;
    vec2 B=P2-P1-A;
    vec2 Ma=P0-M;
    float a=dot(B,B);
    float b=3*dot(A,B);
    float c=2*dot(A,A)+dot(Ma,B);
    float d=dot(Ma,A);


    // 处理退化情况
    if (abs(a) < EPSILON)  // 不是三次方程
    {
        if (abs(b) < EPSILON)  // 一次方程 cx + d = 0
        {
            if (abs(c) < EPSILON)// 无解或无穷解
            {
            // 丢弃这个case，设置为无穷大，这个点的距离会被其他最小距离代替
            // 如果真的存在这个case,会表现为这个点突起
                return INF;
            }
            float t= -d/c;
            if (t < 0.0 || t >= 1.0) // t不在[0,1]区间
            {
            // 与曲线的切点不在曲线上，最近点为两个端点中最近的点
                float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                return min(dist1, dist2);

            }
            vec2 point= QuadraticBezier(P0, P1, P2, t);
            return length(point - M);
        }

        else  // 二次方程 bx² + cx + d = 0
        {
            float discriminant = c*c - 4*b*d;
            if (discriminant < 0) //无解
            {
                return INF; // 丢弃这个case
            }
            else if (discriminant == 0)//单根
            {
                float t = -c/(2*b);
                if (t < 0.0 || t >= 1.0) // t不在[0,1]区间
                {
                    // 与曲线的切点不在曲线上，最近点为两个端点中最近的点
                float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                return min(dist1, dist2);
                }
                vec2 point = QuadraticBezier(P0, P1, P2, t);
                return length(point - M);
            }
            else // 两个根
            {
                float sqrt_disc = sqrt(discriminant);
                float t1=(-c + sqrt_disc)/(2*b);
                float t2=(-c - sqrt_disc)/(2*b);
                float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                float minDist= min(dist1, dist2);
                if (t1 >= 0.0 && t1 < 1.0) {
                    vec2 point1 = QuadraticBezier(P0, P1, P2, t1);
                    minDist = length(point1 - M);
                }
                if (t2 >= 0.0 && t2 < 1.0) {
                    vec2 point2 = QuadraticBezier(P0, P1, P2, t2);
                    float dist2 = length(point2 - M);
                    if (dist2 < minDist) {
                        minDist = dist2;
                    }
                }
                return minDist;
            }
                
        }

    }
    // 处理非退化情况
    // 三次方程求解 - 使用Cardano公式
    // 先转换为标准形式 t³ + pt + q = 0
    float p = (3*a*c - b*b) / (3*a*a);
    float q = (2*b*b*b - 9*a*b*c + 27*a*a*d) / (27*a*a*a);
    
    float discriminant = pow(q/2.0,2.0) + pow(p/3.0,3.0);
    
    
    if (discriminant > 0)
    {
        //一个实根
        float sqrt_disc = sqrt(discriminant);
        float u;
        if((-q/2.0 + sqrt_disc) >= 0)
        {
            u=pow(-q/2.0 + sqrt_disc,1.0/3.0);
        }
        else
        {
            u=-pow(abs(-q/2.0 + sqrt_disc),1.0/3.0);
        }
       
        float v;
        if((-q/2.0 - sqrt_disc) >= 0)
        {
            v=pow(-q/2.0 - sqrt_disc,1.0/3.0);
        }
        else
        {
            v=-pow(abs(-q/2.0 - sqrt_disc),1.0/3.0);
        }
        float x = u + v;
        float t = x - b/(3*a);
        if (t < 0.0 || t >= 1.0) // t不在[0,1]区间
        {
            // 与曲线的切点不在曲线上，最近点为两个端点中最近的点
                float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                return min(dist1, dist2);
        }
        vec2 point = QuadraticBezier(P0, P1, P2, t);
        return length(point - M);
    }
        
    else if (discriminant == 0)
    {
    // 两个或三个实根
        if (abs(q) < EPSILON)
        {
            // 三重根
            float t = -b/(3*a);
            if (t < 0.0 || t >= 1.0) // t不在[0,1]区间
            {
                // 与曲线的切点不在曲线上，最近点为两个端点中最近的点
                float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                return min(dist1, dist2);
            }
            vec2 point = QuadraticBezier(P0, P1, P2, t);
            return length(point - M);
        }
            
        else
        {
            // 一个单根，一个二重根
            float x1;
            if(abs(p)>EPSILON)
            {
                x1 = 3*q/p;
            }
            else
            {
                x1 = 0;
            }
            float x2;
            if(abs(p)>EPSILON)
            {
                x2 = -3*q/(2*p);
            }
            else
            {
                x2 = 0;
            }
            float t1 = x1 - b/(3*a);
            float t2 = x2 - b/(3*a);
            float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                float minDist= min(dist1, dist2);
            if (t1 >= 0.0 && t1 < 1.0) {
                vec2 point1 = QuadraticBezier(P0, P1, P2, t1);
                minDist = length(point1 - M);
            }
            if (t2 >= 0.0 && t2 < 1.0) {
                vec2 point2 = QuadraticBezier(P0, P1, P2, t2);
                float dist2 = length(point2 - M);
                if (dist2 < minDist) {
                    minDist = dist2;
                }
            }
            return minDist;
        }
            
    }
        
    else
    {
    // 三个不同实根
        float rho = sqrt(pow(-(p/3),3.0));
        float theta = acos(-q/(2*rho));
                float dist1 = length(P0 - M);
                float dist2 = length(P2 - M);
                float minDist= min(dist1, dist2);
        for(int k=0;k<3;++k)
        {
            float x = 2 * (pow(rho,1.0/3.0)) * cos((theta + 2*k*PI)/3.0);
            float t = x - b/(3*a);
            if (t < 0.0 || t >= 1.0) // t不在[0,1]区间
            {
                continue;
            }
            vec2 point = QuadraticBezier(P0, P1, P2, t);
            float dist = length(point - M);
            if (dist < minDist) {
                minDist = dist;
            }
            
        }
        return minDist;
    }
}
void Sdf()
{
    float minDist = INF;
    Glyph glyph = FetchGlyph(v_GlyphIndex);
    for (int i = 0; i < glyph.count; i++) {
		Curve curve = FetchCurve(glyph.start+i);
        float dist=NearestToBezier(curve.p0, curve.p1, curve.p2, v_UV);
        if (dist < minDist) {
            minDist = dist;
        }
	}
    minDist=minDist*5;
    FragColor=vec4(minDist, minDist, minDist, 1.0);
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
layout(location=0)in vec3 a_Position;//屏幕坐标系,glyph的aabb
layout(location=1)in uint a_GlyphIndex;
layout(location=2)in vec2 a_UV;// 在归一化的em坐标系
layout(std140,binding=0)uniform UniformBufferObject
{
    mat4 u_MVP;// 只有view和projection矩阵,model矩阵是identity
    vec4 u_Color;
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
    
    float worldSize = param.worldSize;
    
    assert(param.bufferGlyphInfoIndexes.size()==param.glyphPosition.size());
    for(size_t i=0;i<param.bufferGlyphInfoIndexes.size();++i)
    {
        auto index=param.bufferGlyphInfoIndexes[i];
        auto& pos=param.glyphPosition[i];
        auto& glyph = param.font.bufferGlyphInfo[index];
        auto& bufferGlyph=param.font.bufferGlyphs[glyph.bufferIndex];
        float emSize = glyph.emSize;
        float scale=worldSize/emSize;
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
    if(mesh.GetMesh().CalculateVertexCount()==0)
    {
        return false;// no glyph to render
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
    Render::Utils::VkDrawMesh(commandBuffer, resource.mesh.GetVk());
    return true;
}
} // namespace Aether::Text