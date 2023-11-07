#include "ShaderTest.h"
#include "Aether/Asset/ModelAsset.h"
#include "Aether/Asset/ModelAssetImporter.h"
#include "Aether/Render/Math.h"
Test::ShaderTest::ShaderTest()
	:m_CameraController(Aether::Camera::CreatePerspectiveCamera(16.0 / 9.0)),
	m_MouseRecord(Eigen::Vector2d(0.0, 0.0))
{
	 
	auto res=Aether::ModelAssetImporter::LoadFromFile(RESOURCE("Model/sphere.fbx"));
	assert(res);
	auto modelAssetRef = Aether::CreateRef<Aether::ModelAsset>(std::move(res.value()));
	modelAssetRef->GetMeshes()[0].CalculateBoundingBox();
	m_Max = modelAssetRef->GetMeshes()[0].PositionMax.value();
	m_Min = modelAssetRef->GetMeshes()[0].PositionMin.value();
	m_ModelMatrix = Aether::Math::Translation((-(m_Max + m_Min) / 2) + Eigen::Vector3f(0.5, 0.5, -0.5));
	m_Model=Aether::CreateRef<Aether::Model>(modelAssetRef);
	m_ModelMatrix = Aether::Math::Identity();
	m_ModelScaling = 0.2f;
	m_ModelTranslation = Eigen::Vector3f({ 0.0f,0.0f,-0.719f });
	m_ModelRotation = Eigen::Vector3f({ 0.0,0.0,0.0 });
	
}

Test::ShaderTest::~ShaderTest()
{
}

void Test::ShaderTest::OnRender()
{
	auto& camera = m_CameraController.GetCamera();
	camera.CalculateViewMatrix();
	camera.CalculateCameraMatrix();
	m_ModelMatrix = Aether::Math::Translation(m_ModelTranslation) *
		Aether::Math::Scale(m_ModelScaling, m_ModelScaling, m_ModelScaling) *
		Aether::Math::RotateX(m_ModelRotation.x()) *
		Aether::Math::RotateY(m_ModelRotation.y()) *
		Aether::Math::RotateZ(m_ModelRotation.z());
	Aether::Renderer::BeginScene(camera);
	if (m_Shader)
	{
		m_Model->Draw(m_Shader,m_ModelMatrix
		);
	}
	Aether::Renderer::EndScene();

}

void Test::ShaderTest::OnImGuiRender()
{
	ImGui::Begin("shader");
	ImGui::Text("shader path: %s", m_ShaderPath.c_str());
	if (!m_Shader)
	{
		ImGui::Text("current shader is null");
	}
	if (ImGui::Button("reload"))
	{
		auto res = Aether::Shader::CreateRefFromFile(m_ShaderPath.c_str());
		if (res)
		{
			m_Shader = res.value();
		}
	}
	ImGui::End();
	ImGui::Begin("RenderParam");
	ImGui::Text("Model Matrix Param");
	ImGui::SliderFloat("scaling", &m_ModelScaling, 0.f, 1.f);
	ImGui::SliderFloat3("translation", m_ModelTranslation.data(), -1.f, 1.f);
	ImGui::SliderFloat3("rotation", m_ModelRotation.data(), -6.28f, 6.28f);
	
	ImGui::End();
}

void Test::ShaderTest::OnEvent(Aether::Event& event)
{
	Aether::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Aether::WindowFileDropEvent>([this](Aether::WindowFileDropEvent& e) {
		return this->OnFileDrop(e);
	});
	m_CameraController.OnEvent(event);
	m_KeyboardRecord.OnEvent(event);
	m_MouseRecord.OnEvent(event);
}

bool Test::ShaderTest::OnFileDrop(Aether::WindowFileDropEvent& event)
{
	m_ShaderPath = event.GetPaths()[0];
	auto res=Aether::Shader::CreateRefFromFile(m_ShaderPath.c_str());
	if (res)
	{
		m_Shader = res.value();
	}
	return true;
}
void Test::ShaderTest::OnLoopEnd()
{
	m_MouseRecord.ClearOffsets();
}
void Test::ShaderTest::CalculateModelMatrix()
{
	m_ModelMatrix = Aether::Math::Translation(m_ModelTranslation) *
		Aether::Math::Scale(m_ModelScaling, m_ModelScaling, m_ModelScaling) *
		Aether::Math::RotateX(m_ModelRotation.x()) *
		Aether::Math::RotateY(m_ModelRotation.y()) *
		Aether::Math::RotateZ(m_ModelRotation.z());
}