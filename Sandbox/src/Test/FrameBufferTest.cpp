#include "FrameBufferTest.h"
#include "Aether/Render/OpenGLApi.h"
#include "Aether/Core/Log.h"
#include "Aether/Render/VertexArray.h"
#include "Aether/Core/Log.h"
#include "Aether/Asset/ModelAssetImporter.h"
Test::FrameBufferTest::FrameBufferTest()
	:m_CameraController(Aether::Camera::CreatePerspectiveCamera(16.0 / 9.0)),
	m_MouseRecord(Eigen::Vector2d(0.0, 0.0))
{
	//初始化模型资源
	auto loadRes = Aether::ModelAssetImporter::LoadFromFile(RESOURCE("Model/just_a_girl.glb"));
	assert(loadRes);
	m_ModelAsset = CreateRef<Aether::ModelAsset>(std::move(loadRes.value()));
	for (auto& mesh : m_ModelAsset->GetMeshes())
	{
		mesh.CalculateBoundingBox();
	}
	m_Model = Aether::CreateRef<Aether::Model>(m_ModelAsset);
	m_Shader = Aether::CreateRef<Aether::Shader>(RESOURCE("Shader/BasicLight.shader"));
	InitRenderParam();
	//初始化quad资源
	float rawQuadVertexBuffer[16] =
	{ 
		//position //uv
		-1, 1,0,1,//0
		 1, 1,1,1,//1
		 1,-1,1,0,//2
		-1,-1,0,0//3
	};
	uint32_t rawIndexBuffer[6] =
	{
		0,1,3,
		1,2,3
	};
	m_QuadVB = Aether::CreateRef<Aether::VertexBuffer>(rawQuadVertexBuffer, 16*sizeof(float));
	m_QuadVBL = Aether::CreateRef<Aether::VertexBufferLayout>();
	m_QuadVBL->Push<float>(2);
	m_QuadVBL->Push<float>(2);
	m_QuadIB = Aether::CreateRef<Aether::IndexBuffer>(rawIndexBuffer, 6);
	m_QuadVA = Aether::CreateRef<Aether::VertexArray>();
	m_QuadVA->SetData(*m_QuadVB, *m_QuadVBL);
	std::string quadShader = 
		"#vertex_shader\n"
		"#version 460 core\n"
		"layout(location = 0) in vec2 a_Position;\n"
		"layout(location = 1) in vec2 a_TexCoords;\n"
		"out vec2 v_TexCoords;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(a_Position,0,1);\n"
		"	v_TexCoords=a_TexCoords;\n"
		"}\n"
		"#fragment_shader\n"
		"#version 460 core\n"
		"out vec4 color;\n"
		"in vec2 v_TexCoords;\n"
		"uniform sampler2D u_ScreenTexture;\n"
		"void main()\n"
		"{\n"
		"	color=1.0-texture(u_ScreenTexture,v_TexCoords);\n"
		"}\n";
	auto res = Aether::Shader::CreateRefFromMem(quadShader.data(), quadShader.size());
	assert(res);
	m_QuadShader = res.value();
	
	//初始化帧缓冲
	glGenFramebuffers(1, &m_Framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
	// 生成纹理
	Aether::OpenGLApi::ActivateTexture(0);
	glGenTextures(1, &m_TexColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_TexColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1600, 900, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// 将它附加到当前绑定的帧缓冲对象
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TexColorBuffer, 0);
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		Aether::Log::Debug("FrameBuffer Complete!");
	}
	else
	{
		Aether::Log::Debug("FrameBuffer Not Complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

Test::FrameBufferTest::~FrameBufferTest()
{
	glDeleteFramebuffers(1, &m_Framebuffer);
	//glDeleteTextures(1, &m_TexColorBuffer);
}






void Test::FrameBufferTest::OnRender()
{

	// 渲染到framebuffer上
	glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

	Aether::OpenGLApi::ClearColorBuffer();
	Aether::OpenGLApi::ClearDepthBuffer();
	
	auto& camera = m_CameraController.GetCamera();
	camera.CalculateViewMatrix();
	camera.CalculateCameraMatrix();

	Aether::Renderer::BeginScene(camera);
	CalculateModelMatrix();
	m_Shader->Bind();
	m_Shader->Bind();
	m_Shader->SetVec3f("u_LightColor", m_LightColor);
	m_Shader->SetVec3f("u_LightDirection", m_LightDirection);
	
	
	m_Model->Draw(m_Shader, m_ModelMatrix);
	Aether::Renderer::EndScene();

	// 在quad上渲染生成的纹理
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Aether::OpenGLApi::ClearColorBuffer();
	Aether::OpenGLApi::ClearDepthBuffer();
	Aether::OpenGLApi::ActivateTexture(0);
	glBindTexture(GL_TEXTURE_2D, m_TexColorBuffer);
	m_QuadShader->Bind();
	
	m_Shader->Unbind();
	m_QuadShader->Bind();
	m_QuadShader->SetInt("u_ScreenTexture", 0);
	Aether::OpenGLApi::DrawElements(*m_QuadVA, *m_QuadIB);
}

void Test::FrameBufferTest::OnImGuiRender()
{
	ImGui::Begin("RenderParam");
	ImGui::Text("Model Matrix Param");
	ImGui::SliderFloat("scaling", &m_ModelScaling, 0.f, 1.f);
	ImGui::SliderFloat3("translation", m_ModelTranslation.data(), -1.f, 1.f);
	ImGui::SliderFloat3("rotation", m_ModelRotation.data(), -6.28f, 6.28f);
	ImGui::Text("Light Param");
	ImGui::SliderFloat3("light color", m_LightColor.data(), 0.f, 1.f);
	ImGui::SliderFloat3("light direction", m_LightDirection.data(), -1.f, 1.f);
	ImGui::End();
}

void Test::FrameBufferTest::OnEvent(Aether::Event& event)
{
	m_CameraController.OnEvent(event);
	m_KeyboardRecord.OnEvent(event);
	m_MouseRecord.OnEvent(event);
}

void Test::FrameBufferTest::OnUpdate(float sec)
{
	if (m_MouseRecord.IsPressed(Aether::MouseButtonCode::MOUSE_BUTTON_MIDDLE))
	{
		for (auto& offset : m_MouseRecord.GetOffsets())
		{
			m_CameraController.GetCamera().Rotate(Aether::Math::RotateX(offset.y() / 1000) * Aether::Math::RotateY(-offset.x() / 1000));
		}
	}
}

void Test::FrameBufferTest::OnLoopEnd()
{
	m_MouseRecord.ClearOffsets();
}

void Test::FrameBufferTest::OnLoopBegin()
{
}

void Test::FrameBufferTest::InitRenderParam()
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

void Test::FrameBufferTest::CalculateModelMatrix()
{
	m_ModelMatrix = Aether::Math::Translation(m_ModelTranslation) *
		Aether::Math::Scale(m_ModelScaling, m_ModelScaling, m_ModelScaling) *
		Aether::Math::RotateX(m_ModelRotation.x()) *
		Aether::Math::RotateY(m_ModelRotation.y()) *
		Aether::Math::RotateZ(m_ModelRotation.z());
}
