#pragma once
#include "Mesh.h"
#include "GpuMesh.h"
namespace Aether
{
    std::expected<GpuMesh,std::string> UploadMesh(const Mesh& mesh);
}