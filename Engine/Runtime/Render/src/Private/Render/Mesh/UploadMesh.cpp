#include <Render/Mesh/UploadMesh.h>
#include <Render/Mesh/CreateMeshVertexLayout.h>
#include <Render/Threads/SubmitThread.h>

namespace Aether
{
    std::expected<GpuMesh,std::string> UploadMesh(const Mesh& mesh)
    {
        GpuMesh gpuMesh;
        auto layoutResultOrErr=CreateMeshVertexLayoutEx(mesh);
        if(!layoutResultOrErr)
        {
            return std::unexpected<std::string>(layoutResultOrErr.error());
        }
        auto& [layout,map]=layoutResultOrErr.value();
        // upload buffers
    }
}