#include "Renderer3D.h"
#include "OpenGLApi.h"
#include "Math.h"
#include "Eigen/Core"
#include <Eigen/Dense>
#include <sstream>
#include "../Core/Log.h"
namespace Aether
{


const Camera* Renderer3D::s_Camera=nullptr;


void Renderer3D::BeginScene(const Camera* camera)
{
	s_Camera = camera;
}

void Renderer3D::EndScene()
{
}

void Submit(Ref<Model> model,const Mat4& modelMatrix)
{
	
	model->Render();
	for(auto& p:model->primitives)
    {
        //get indices cnt
        auto opt_accessorIndex=p.GetIndices();
        AETHER_ASSERT(opt_accessorIndex&&"no indices draw not implment");
        auto& indicesAccessor=model->accessors[opt_accessorIndex.value()];
        auto& indicesBufferView=model->bufferViews[indicesAccessor.GetBufferView()];
        size_t indicesCnt=(indicesBufferView.GetEnd()-indicesBufferView.GetBegin())/
        indicesAccessor.GetStride();
        //draw
        OpenGLApi::DrawElements(p.GetVertexArray(), *indicesBufferView.GetIndexBuffer(), indicesCnt);
        
    }
}

}//namespace Aether