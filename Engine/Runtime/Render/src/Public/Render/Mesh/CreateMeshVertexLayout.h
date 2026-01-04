#pragma once
#include "Mesh.h"
#include "VertexLayout.h"
#include <expected>
namespace Aether
{
    struct GpuMeshToCpuMeshMap
    {
        std::unordered_map<uint32_t,uint32_t> bufferMap; // gpu buffer index -> cpu buffer index
        std::unordered_map<uint32_t,uint32_t> bufferViewMap; // gpu bufferView index -> cpu bufferView index
    };
    std::expected<std::pair<VertexLayout,GpuMeshToCpuMeshMap>,std::string> CreateMeshVertexLayoutEx(const Mesh& mesh);
    std::expected<VertexLayout,std::string> CreateMeshVertexLayout(const Mesh& mesh);
}