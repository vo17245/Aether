#include "Renderer3D.h"
#include "OpenGLApi.h"
#include "Math.h"
#include "Eigen/Core"
#include <Eigen/Dense>
#include "ShaderLanguage.h"
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

}//namespace Aether