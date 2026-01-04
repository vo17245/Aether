#include <Render/Mesh/CreateMeshVertexLayout.h>
#include <algorithm>
namespace Aether
{
    static VertexAttributeFormat MeshFormatToVertexAttributeFormat(Mesh::Type type,Mesh::ComponentType componentType)
    {
        if(componentType==Mesh::ComponentType::FLOAT32)
        {
            switch(type)
            {
            case Mesh::Type::SCALAR:
                return VertexAttributeFormat::Float32;
            case Mesh::Type::VEC2:
                return VertexAttributeFormat::Vec2f;
            case Mesh::Type::VEC3:
                return VertexAttributeFormat::Vec3f;
            case Mesh::Type::VEC4:
                return VertexAttributeFormat::Vec4f;
            default:
                return VertexAttributeFormat::None;
            }
        }
        else if(componentType==Mesh::ComponentType::UINT32)
        {
            if(type==Mesh::Type::SCALAR)
            {
                return VertexAttributeFormat::UInt32;
            }
        }
        return VertexAttributeFormat::None;
    }
    static size_t GetVertexAttributeFormatSize(VertexAttributeFormat format)
    {
        switch(format)
        {
        case VertexAttributeFormat::Float32:
            return 4;
        case VertexAttributeFormat::Vec2f:
            return 8;
        case VertexAttributeFormat::Vec3f:
            return 12;
        case VertexAttributeFormat::Vec4f:
            return 16;
        case VertexAttributeFormat::UInt32:
            return 4;
        default:
            return 0;
        }
    }
    std::expected<std::pair<VertexLayout,GpuMeshToCpuMeshMap>,std::string> CreateMeshVertexLayoutEx(const Mesh& mesh)
    {
        GpuMeshToCpuMeshMap map;
        std::unordered_map<uint32_t,uint32_t> bufferMap;// mesh buffer index -> gpu buffer index
        std::unordered_map<uint32_t,uint32_t> bufferViewMap;// mesh bufferView index -> gpu bufferView index
        VertexLayout layout;
        auto& primitive=mesh.primitive;
        std::unordered_map<uint32_t,uint32_t> bufferViewStrides;//mesh bufferView index -> stride
        for(const auto& [attribute,accessorIndex]:mesh.primitive.attributes)
        {
            VertexAttribute vertexAttribute;
            if(accessorIndex>=mesh.accessors.size())
            {
                return std::unexpected<std::string>("CreateMeshVertexLayout: accessor index out of range");
            }
            auto& accessor=mesh.accessors[accessorIndex];
            if(accessor.bufferView>=mesh.bufferViews.size())
            {
                return std::unexpected<std::string>("CreateMeshVertexLayout: bufferView index out of range");
            }
            auto& bufferView=mesh.bufferViews[accessor.bufferView];
            uint32_t gpuBufferViewIndex;
            {
                auto iter=bufferViewMap.find(accessor.bufferView);
                if(iter==bufferViewMap.end())
                {
                    gpuBufferViewIndex=static_cast<uint32_t>(bufferViewMap.size());
                    bufferViewMap[accessor.bufferView]=gpuBufferViewIndex;
                    // map buffer
                    if(bufferView.buffer>=mesh.buffers.size())
                    {
                        return std::unexpected<std::string>("CreateMeshVertexLayout: buffer index out of range");
                    }
                    auto bufferIter=bufferMap.find(bufferView.buffer);
                    if(bufferIter==bufferMap.end())
                    {
                        uint32_t gpuBufferIndex=static_cast<uint32_t>(bufferMap.size());
                        bufferMap[bufferView.buffer]=gpuBufferIndex;
                    }   

                }
                else
                {
                    gpuBufferViewIndex=iter->second;
                }
            }

            vertexAttribute.bufferIndex=gpuBufferViewIndex;
            vertexAttribute.offset=accessor.byteOffset;
            auto format=MeshFormatToVertexAttributeFormat(accessor.type,accessor.componentType);
            if(format==VertexAttributeFormat::None)
            {
                return std::unexpected<std::string>("CreateMeshVertexLayout: unsupported attribute format");
            }
            vertexAttribute.format=format;
            layout.attributes.push_back(vertexAttribute);
            if(bufferViewStrides.find(accessor.bufferView)==bufferViewStrides.end())
            {
                bufferViewStrides[accessor.bufferView]=0;
            }
            size_t attributeSize=GetVertexAttributeFormatSize(format);
            size_t attributeEnd=accessor.byteOffset+attributeSize;
            if(attributeEnd>bufferViewStrides[accessor.bufferView])
            {
                bufferViewStrides[accessor.bufferView]=attributeEnd;
            }
        }
        // set strides
        layout.buffers.resize(bufferViewStrides.size());
        for(const auto& [meshBufferViewIndex,stride]:bufferViewStrides)
        {
            uint32_t gpuBufferViewIndex=bufferViewMap[meshBufferViewIndex];
            layout.buffers[gpuBufferViewIndex].stride=static_cast<uint32_t>(stride);
        }
        for(auto& [meshBufferIndex,gpuBufferIndex]:bufferMap)
        {
            map.bufferMap[gpuBufferIndex]=meshBufferIndex;
        }
        for(auto& [meshBufferViewIndex,gpuBufferViewIndex]:bufferViewMap)
        {
            map.bufferViewMap[gpuBufferViewIndex]=meshBufferViewIndex;
        }
        return std::make_pair(layout,map);
    }
    std::expected<VertexLayout,std::string> CreateMeshVertexLayout(const Mesh& mesh)
    {
        auto resOrErr=CreateMeshVertexLayoutEx(mesh);
        if(!resOrErr)
        {
            return std::unexpected<std::string>(resOrErr.error());
        }
        return resOrErr->first;
    }
}