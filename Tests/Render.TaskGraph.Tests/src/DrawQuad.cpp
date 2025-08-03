#include "DrawQuad.h"
#include <Render/Utils.h>
static DeviceRenderPass CreateRenderPass(PixelFormat colorAttachPixelFormat,bool clear)
{
    DeviceRenderPassDesc desc;
    desc.colorAttachmentCount=1;
    desc.colorAttachments[0].format=colorAttachPixelFormat;
    if(clear)
    {
        desc.colorAttachments[0].loadOp=DeviceAttachmentLoadOp::Clear;
    }
    else  
    {
        desc.colorAttachments[0].loadOp=DeviceAttachmentLoadOp::Clear;
    }
    desc.colorAttachments[0].storeOp=DeviceAttachmentStoreOp::Store;
    return DeviceRenderPass::Create(desc);
}
static Mesh CreateMesh(const Mat2x3f& affine)
{
    std::array<Vec2f,4> positions={
        Vec2f(-1.0f, -1.0f),
        Vec2f(1.0f, -1.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(-1.0f, 1.0f)
    };
    std::array<Vec2f,4> uvs={
        Vec2f(0.0f, 0.0f),
        Vec2f(1.0f, 0.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(0.0f, 1.0f)
    };
    std::array<uint32_t,6> indices={
        0,1,2,
        0,2,3
    };
    Eigen::Matrix<float, 2, 3> _affine=affine;
    
    for(auto& p:positions)
    {

        p=_affine*Vec3f(p.x(),p.y(),1.0f);
    }
    Mesh mesh;
    size_t positionBufferIndex=mesh.buffers.size();
    mesh.buffers.emplace_back();
    mesh.buffers[positionBufferIndex].insert(mesh.buffers[positionBufferIndex].end(),
        reinterpret_cast<const uint8_t*>(positions.data()),
        reinterpret_cast<const uint8_t*>(positions.data()) + positions.size() * sizeof(Vec2f));
    size_t uvBufferIndex=mesh.buffers.size();
    mesh.buffers.emplace_back();
    mesh.buffers[uvBufferIndex].insert(mesh.buffers[uvBufferIndex].end(),
        reinterpret_cast<const uint8_t*>(uvs.data()),
        reinterpret_cast<const uint8_t*>(uvs.data()) + uvs.size() * sizeof(Vec2f));
    size_t indexBufferIndex=mesh.buffers.size();
    mesh.buffers.emplace_back();
    mesh.buffers[indexBufferIndex].insert(mesh.buffers[indexBufferIndex].end(),
        reinterpret_cast<const uint8_t*>(indices.data()),
        reinterpret_cast<const uint8_t*>(indices.data()) + indices.size() * sizeof(uint32_t));
    size_t positionBufferViewIndex=mesh.bufferViews.size();
    mesh.bufferViews.emplace_back();
    mesh.bufferViews[positionBufferIndex].offset=0;
    mesh.bufferViews[positionBufferIndex].size=static_cast<uint32_t>(positions.size() * sizeof(Vec2f));
    mesh.bufferViews[positionBufferIndex].buffer=positionBufferIndex;
    mesh.bufferViews[positionBufferIndex].target=Mesh::Target::Vertex;
    size_t uvBufferViewIndex=mesh.bufferViews.size();
    mesh.bufferViews.emplace_back();
    mesh.bufferViews[uvBufferIndex].offset=0;
    mesh.bufferViews[uvBufferIndex].size=static_cast<uint32_t>(uvs.size() * sizeof(Vec2f));
    mesh.bufferViews[uvBufferIndex].buffer=uvBufferIndex;
    mesh.bufferViews[uvBufferIndex].target=Mesh::Target::Vertex;
    size_t indexBufferViewIndex=mesh.bufferViews.size();
    mesh.bufferViews.emplace_back();
    mesh.bufferViews[indexBufferIndex].offset=0;
    mesh.bufferViews[indexBufferIndex].size=static_cast<uint32_t>(indices.size() * sizeof(uint32_t));
    mesh.bufferViews[indexBufferIndex].buffer=indexBufferIndex;
    mesh.bufferViews[indexBufferIndex].target=Mesh::Target::Index;
    size_t positionAccessorIndex=mesh.accessors.size();
    mesh.accessors.emplace_back();
    mesh.accessors[positionAccessorIndex].bufferView=positionBufferViewIndex;
    mesh.accessors[positionAccessorIndex].byteOffset=0;
    mesh.accessors[positionAccessorIndex].componentType=Mesh::ComponentType::FLOAT32;
    mesh.accessors[positionAccessorIndex].count=static_cast<uint32_t>(positions.size());
    mesh.accessors[positionAccessorIndex].type=Mesh::Type::VEC2;
    size_t uvAccessorIndex=mesh.accessors.size();
    mesh.accessors.emplace_back();
    mesh.accessors[uvAccessorIndex].bufferView=uvBufferViewIndex;
    mesh.accessors[uvAccessorIndex].byteOffset=0;
    mesh.accessors[uvAccessorIndex].componentType=Mesh::ComponentType::FLOAT32;
    mesh.accessors[uvAccessorIndex].count=static_cast<uint32_t>(uvs.size());
    mesh.accessors[uvAccessorIndex].type=Mesh::Type::VEC2;
    size_t indexAccessorIndex=mesh.accessors.size();
    mesh.accessors.emplace_back();
    mesh.accessors[indexAccessorIndex].bufferView=indexBufferViewIndex;
    mesh.accessors[indexAccessorIndex].byteOffset=0;
    mesh.accessors[indexAccessorIndex].componentType=Mesh::ComponentType::UINT32;
    mesh.accessors[indexAccessorIndex].count=static_cast<uint32_t>(indices.size());
    mesh.accessors[indexAccessorIndex].type=Mesh::Type::SCALAR;
    mesh.primitive.attributes[Mesh::Attribute::POSITION] = positionAccessorIndex;
    mesh.primitive.attributes[Mesh::Attribute::TEXCOORD] = uvAccessorIndex;
    mesh.primitive.index = indexAccessorIndex;
    return mesh;
}
std::expected<DrawQuad, std::string> DrawQuad::Create(const std::string& vs,const std::string& fs,const Mat2x3f& affine,
    PixelFormat colorAttachPixelFormat,bool clear)
{
    DrawQuad drawQuad;
    // create shader
    auto shaderEx = DeviceShader::Create(ShaderSource(ShaderStageType::Vertex, ShaderLanguage::GLSL, vs),
                                         ShaderSource(ShaderStageType::Fragment, ShaderLanguage::GLSL, fs));
    if (!shaderEx)
    {
        return std::unexpected<std::string>(shaderEx.error());
    }
    auto shader = std::move(shaderEx.value());
    // create render pass
    auto renderPass = CreateRenderPass(colorAttachPixelFormat, clear);
    if (renderPass.Empty())
    {
        return std::unexpected<std::string>("failed to create render pass");
    }
    // create pipeline
    vk::PipelineLayout::Builder layoutBuilder;
    auto layoutOpt = layoutBuilder.Build();
    if (!layoutOpt)
    {
        return std::unexpected<std::string>("failed to create pipeline layout");
    }
    auto& layout = layoutOpt.value();
    vk::GraphicsPipeline::Builder builder(renderPass.GetVk(), layout);
    Mesh mesh=CreateMesh(affine);
    auto vertexBufferLayouts = mesh.CreateVertexBufferLayouts();
    builder.PushVertexBufferLayouts(vertexBufferLayouts)
        .AddVertexStage(shader.GetVk().vertex, "main")
        .AddFragmentStage(shader.GetVk().fragment, "main");
        builder.EnableBlend();
    auto pipelineOpt = builder.Build();
    if (!pipelineOpt)
    {
        return std::unexpected<std::string>("failed to create graphics pipeline");
    }
    auto& pipeline = pipelineOpt.value();

    drawQuad.m_Pipeline = std::move(pipeline);
    drawQuad.m_PipelineLayout = std::move(layout);
    drawQuad.m_Mesh=DeviceMesh::Create(mesh);
    return drawQuad;
}
void DrawQuad::Render(DeviceCommandBufferView& commandBuffer)
{
    auto& cb=commandBuffer.GetVk();
    cb.BindPipeline(m_Pipeline.GetVk());
    cb.SetScissor(0, 0, m_ScreenSize.x(), m_ScreenSize.y());
    cb.SetViewport(0, 0, m_ScreenSize.x(), m_ScreenSize.y());
    Render::Utils::DrawMesh(cb, m_Mesh.GetVk());
    
}