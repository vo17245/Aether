#include "ShowModelTest.h"
#include "Aether/Render/VertexArray.h"
#include "Aether/Core/Log.h"
Test::ShowModelTest::ShowModelTest()
	:m_CameraController(Aether::Camera::CreatePerspectiveCamera(16.0/9.0)),
	m_MouseRecord(Eigen::Vector2d(0.0,0.0))
{
	m_ModelAsset = Aether::CreateRef<Aether::ModelAsset>(RESOURCE("Model/just_a_girl.glb"));
	for (auto& mesh : m_ModelAsset->GetMeshes())
	{
		mesh.CalculateBoundingBox();
	}
	m_Model = Aether::CreateRef<Aether::Model>(m_ModelAsset);
	m_Shader = Aether::CreateRef<Aether::Shader>(RESOURCE("Shader/BasicLight.shader"));
	InitRenderParam();
}

Test::ShowModelTest::ShowModelTest(const std::string& modelPath)
	:m_CameraController(Aether::Camera::CreatePerspectiveCamera(16.0 / 9.0)), m_MouseRecord(Eigen::Vector2d(0.0, 0.0))
{
	m_ModelAsset = Aether::CreateRef<Aether::ModelAsset>(modelPath);
	m_Model = Aether::CreateRef<Aether::Model>(m_ModelAsset);
	m_Shader = Aether::CreateRef<Aether::Shader>(RESOURCE("Shader/Basic.shader"));
	InitRenderParam();
}

Test::ShowModelTest::ShowModelTest(Aether::Ref<Aether::ModelAsset> modelAsset)
	:m_CameraController(Aether::Camera::CreatePerspectiveCamera(16.0 / 9.0)), m_MouseRecord(Eigen::Vector2d(0.0, 0.0))
{
	m_Model = Aether::CreateRef<Aether::Model>(modelAsset);
	m_Shader = Aether::CreateRef<Aether::Shader>(RESOURCE("Shader/Basic.shader"));
	InitRenderParam();
}

Test::ShowModelTest::~ShowModelTest()
{
}

void Test::ShowModelTest::OnRender()
{
	auto& camera = m_CameraController.GetCamera();
	camera.CalculateViewMatrix();
	camera.CalculateCameraMatrix();
	Aether::Renderer::BeginScene(camera);
	CalculateModelMatrix();
	m_Shader->Bind();
	m_Shader->SetVec3f("u_LightColor", m_LightColor);
	m_Shader->SetVec3f("u_LightDirection", m_LightDirection);
	m_Model->Draw(m_Shader, m_ModelMatrix);
	Aether::Renderer::EndScene();
}

void Test::ShowModelTest::OnImGuiRender()
{
	ImGui::Begin("RenderParam");
	ImGui::Text("Model Matrix Param");
	ImGui::SliderFloat("scaling", &m_ModelScaling,0.f,1.f);
	ImGui::SliderFloat3("translation", m_ModelTranslation.data(),-1.f, 1.f);
	ImGui::SliderFloat3("rotation", m_ModelRotation.data(), -6.28f, 6.28f);
	ImGui::Text("Light Param");
	ImGui::SliderFloat3("light color", m_LightColor.data(), 0.f, 1.f);
	ImGui::SliderFloat3("light direction", m_LightDirection.data(), -1.f, 1.f);
	ImGui::End();
}

void Test::ShowModelTest::OnEvent(Aether::Event& event)
{
	m_CameraController.OnEvent(event);
	m_KeyboardRecord.OnEvent(event);
	m_MouseRecord.OnEvent(event);
}

void Test::ShowModelTest::OnUpdate(float sec)
{
	if (m_MouseRecord.IsPressed(Aether::MouseButtonCode::MOUSE_BUTTON_MIDDLE))
	{
		for (auto& offset : m_MouseRecord.GetOffsets())
		{
				m_CameraController.GetCamera().Rotate(Aether::Math::RotateX(offset.y() / 1000) * Aether::Math::RotateY(-offset.x() / 1000));
		}
	}
}

void Test::ShowModelTest::OnLoopEnd()
{
	m_MouseRecord.ClearOffsets();
}

void Test::ShowModelTest::InitRenderParam()
{
	// init model matrix
	m_ModelMatrix = Aether::Math::Identity();
	m_ModelScaling = 1.0f;
	m_ModelTranslation = Eigen::Vector3f({ 0.0f,0.0f,0.0f });
	m_ModelRotation = Eigen::Vector3f({ 0.0,0.0,0.0 });
	//init light
	m_LightColor = Eigen::Vector3f({ 1.0,1.0,1.0 });
	m_LightDirection = Eigen::Vector3f({ 0.0,-1.0,0.0 });
}

void Test::ShowModelTest::CalculateModelMatrix()
{
	m_ModelMatrix = Aether::Math::Translation(m_ModelTranslation) * 
		Aether::Math::Scale(m_ModelScaling, m_ModelScaling, m_ModelScaling)*
		Aether::Math::RotateX(m_ModelRotation.x())*
		Aether::Math::RotateY(m_ModelRotation.y())*
		Aether::Math::RotateZ(m_ModelRotation.z());
}
