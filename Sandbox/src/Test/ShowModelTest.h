#pragma once 
#include "Aether/Core/Core.h"
#include "Test.h"
#include <string>
#include "Aether/Asset/ModelAsset.h"
#include "Aether/UI/CameraController.h"
#include "Aether/Asset/Model.h"
#include "Aether/Render/Math.h"
#include "Aether/UI/KeyboardRecord.h"
#include "Aether/UI/MouseRecord.h"
namespace Test
{
class ShowModelTest:public Test
{
public:
	ShowModelTest();
	ShowModelTest(const std::string& modelPath);
	ShowModelTest(Aether::Ref<Aether::ModelAsset> modelAsset);
	~ShowModelTest();
	void OnRender() override;
	void OnImGuiRender() override;
	void OnEvent(Aether::Event& event) override;
	void OnUpdate(float sec) override;
	void OnLoopEnd()override;
private:
	Aether::Ref<Aether::ModelAsset> m_ModelAsset;
	Aether::Ref<Aether::Model> m_Model;
	
	Aether::Ref<Aether::Shader> m_Shader;

private:
	//light
	Eigen::Vector3f m_LightColor;
	Eigen::Vector3f m_LightDirection;


private:
	// model matrix
	Eigen::Matrix4f m_ModelMatrix;
	Eigen::Vector3f m_ModelTranslation;
	float m_ModelScaling;
	Eigen::Vector3f m_ModelRotation;
	void InitRenderParam();
	void CalculateModelMatrix();
private:
	//ui
	Aether::CameraController m_CameraController;
	Aether::MouseRecord m_MouseRecord;
	Aether::KeyboardRecord m_KeyboardRecord;
};
}