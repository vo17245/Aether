#include "ShaderTest.h"
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Asset/ModelAssetImporter.h"
#include "Aether/Render/Math.h"
Test::ShaderTest::ShaderTest()
{
	 
	
	
}

Test::ShaderTest::~ShaderTest()
{
}

void Test::ShaderTest::OnRender()
{
	

}

void Test::ShaderTest::OnImGuiRender()
{
	ImGui::Begin("shader");
	
	
	ImGui::End();
}

void Test::ShaderTest::OnEvent(Aether::Event& event)
{
	
}

bool Test::ShaderTest::OnFileDrop(Aether::WindowFileDropEvent& event)
{
	return false;
}
void Test::ShaderTest::OnLoopEnd()
{
	
}
