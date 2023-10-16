#include "LoadModelTest.h"
#include "Aether/Asset/Model.h"
#include "Aether/Core/Log.h"
static bool IsBufferSame(const void* a, const void* b, size_t byteLen)
{
	char* pa=(char*)a;
	char* pb = (char*)b;
	for (size_t i = 0;i < byteLen;i++)
	{
		if (*(pa + i)!= *(pb + i))
			return false;
	}
	return true;
}
static bool IsMeshSame(const Aether::MeshAsset& a, const Aether::MeshAsset& b)
{
	if (a.Vertices.size() != b.Vertices.size())
		return false;
	return IsBufferSame(a.Vertices.data(), b.Vertices.data(), a.Vertices.size() * sizeof(Aether::Vertex));
	
}
Test::LoadModelTest::LoadModelTest()
{
	Aether::Log::Debug("LoadModelTest Begin");
	std::string path(RESOURCE("Model/cube.glb"));
	auto  modelAsset1=Aether::CreateRef<Aether::ModelAsset>(path);
	assert(modelAsset1->GetMeshes().size() == 1);
	auto  modelAsset2 = Aether::CreateRef<Aether::ModelAsset>(path);
	assert(modelAsset2->GetMeshes().size() == 1);
	auto  modelAsset3 = Aether::CreateRef<Aether::ModelAsset>(path);
	assert(modelAsset3->GetMeshes().size() == 1);
	auto  modelAsset4 = Aether::CreateRef<Aether::ModelAsset>(path);
	assert(modelAsset4->GetMeshes().size() == 1);
	Aether::Log::Debug("LoadModelTest End");
	assert(IsMeshSame(modelAsset1->GetMeshes()[0], modelAsset2->GetMeshes()[0]));
	assert(IsMeshSame(modelAsset1->GetMeshes()[0], modelAsset3->GetMeshes()[0]));
	assert(IsMeshSame(modelAsset1->GetMeshes()[0], modelAsset4->GetMeshes()[0]));

	Aether::Model model1(modelAsset1);
	Aether::Model model2(modelAsset2);
	Aether::Model model3(modelAsset3);
	Aether::Model model4(modelAsset4);
}
