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
#include "Aether/Event/WindowEvent.h"
namespace Test
{
	class ShaderTest :public Test
	{
	public:
		ShaderTest();
		~ShaderTest();
		void OnRender() override;
		void OnImGuiRender() override;
		void OnEvent(Aether::Event& event) override;
		bool OnFileDrop(Aether::WindowFileDropEvent& event);
		void OnLoopEnd()override;
	private:
		Aether::Ref<Aether::Shader> m_Shader;
		std::string m_ShaderPath;
		Aether::Ref<Aether::Model> m_Model;
		Eigen::Vector3f m_Max;
		Eigen::Vector3f m_Min;
		
	private:
		//ui
		Aether::CameraController m_CameraController;
		Aether::MouseRecord m_MouseRecord;
		Aether::KeyboardRecord m_KeyboardRecord;
	private:
		// model matrix
		Eigen::Matrix4f m_ModelMatrix;
		Eigen::Vector3f m_ModelTranslation;
		float m_ModelScaling;
		Eigen::Vector3f m_ModelRotation;
		
		void CalculateModelMatrix();
	};
}