#include "SceneViewPanel.h"
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Scene.h"
#include "Aether/Scene/SceneRenderer.h"
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
            float width_scale=float(windowSize.x)/m_Width;
            float height_scale=float(windowSize.y)/m_Height;
            float image_scale=std::max(width_scale, height_scale);
            ImGui::Image((void*)(intptr_t)
            m_FrameBuffer->GetColorBuffer()->GetRendererId(), 
            ImVec2(m_Width*image_scale, m_Height*image_scale));
            ImGui::End();
        }
        void SceneViewPanel::OnRender()
        {
            m_FrameBuffer->Bind();
            //render scene
            SceneRenderer::Get().Render(MainScene::GetInstance().GetScene(),
            m_CameraController.GetCamera());
            m_FrameBuffer->Unbind();
        }
        void SceneViewPanel::OnUpdate(Real ds)
        {
            m_CameraController.OnUpdate(ds);
        }
    }
}