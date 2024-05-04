#include "SceneViewPanel.h"
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Scene.h"
#include "Core/MainScene.h"
#include <algorithm>
namespace Aether
{
    namespace Editor
    {
        SceneViewPanel::SceneViewPanel()
            :m_CameraController(Math::PI / 4, 0.1, 1000, 1)
        {
            //create framebuffer for scene rendering
            FrameBuffer::Spec spec;
            spec.width=m_Width;
            spec.height=m_Height;
            spec.colorBufferSpec.storage=FrameBuffer::ColorBufferStorage::TEXTURE;
            spec.colorBufferSpec.dataType=FrameBuffer::ColorBufferScalarType::FLOAT32;
            spec.colorBufferSpec.format=FrameBuffer::ColorBufferFormat::RGB;
            spec.colorBufferSpec.internalFormat=FrameBuffer::ColorBufferFormat::RGB;
            m_FrameBuffer=CreateRef<FrameBuffer>(spec);
            
        }
        void SceneViewPanel::OnImGuiRender() 
        {
            int window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse|
             ImGuiWindowFlags_NoScrollbar ;
            ImGui::Begin("Scene View",nullptr,window_flags);
            ImVec2 windowSize = ImGui::GetWindowSize();
            m_PanelHeight = windowSize.y;
            m_PanelWidth = windowSize.x;
            ImGui::SetCursorPos({ 0, 0 });
            ImGui::Image((void*)(intptr_t)
            m_FrameBuffer->GetColorBuffer()->GetRendererId(), 
                windowSize);
            ImGui::End();
        }
        void SceneViewPanel::OnRender()
        {
            m_FrameBuffer->Bind();
            MainScene::GetInstance().OnRender();
            m_FrameBuffer->Unbind();
        }
        void SceneViewPanel::OnUpdate(Real ds)
        {
            m_CameraController.OnUpdate(ds);
        }
    }
}