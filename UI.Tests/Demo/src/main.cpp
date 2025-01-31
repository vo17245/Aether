#include "Render/Vulkan/DescriptorSet.h"
#include "Render/Vulkan/GraphicsCommandPool.h"
#include "Render/Vulkan/Pipeline.h"
#include "Render/Vulkan/PipelineLayout.h"
#include "Render/Vulkan/RenderContext.h"
#include "Render/Vulkan/ShaderModule.h"
#include "UI/Quad.h"
#include "Window/Layer.h"
#include "Window/Window.h"
#include "Window/WindowContext.h"
#include "Render/Vulkan/GlobalRenderContext.h"
#include "Render/Mesh/VkMesh.h"
#include "Render/Mesh/Geometry.h"
#include "Render/Shader/Compiler.h"
#include "Render/Utils.h"
#include <print>
#include "UI/Renderer.h"
using namespace Aether;
class TestLayer : public Layer
{
public:
    
    virtual void OnRender(
        vk::RenderPass& renderPass,
        vk::FrameBuffer& framebuffer,
        vk::GraphicsCommandBuffer& commandBuffer) override
    {
        m_Renderer->Begin(renderPass, framebuffer, Vec2f(800, 600));
        for(auto& quad:m_Quads)
        {
            m_Renderer->DrawQuad(quad);
        }
        m_Renderer->End(commandBuffer);
    }
    virtual void OnAttach(Window* window) override
    {
        //  create renderer
        auto renderer=UI::Renderer::Create(window->GetRenderPass(0));
        if(!renderer)
        {
            std::cout<<renderer.error()<<std::endl;
            return;
        }

        m_Renderer=CreateScope<UI::Renderer>(std::move(renderer.value()));
        m_Renderer->SetScreenSize(Vec2f(800,600));
        // create quads
        UI::QuadDesc desc;
        desc.color=Vec4f(1,0,0,1);
        desc.position=Vec2f(100,100);
        desc.size=Vec2f(100,100);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y()+=110;
        desc.color=Vec4f(0,1,0,1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.x()+=110;
        desc.color=Vec4f(0,0,1,1);
        m_Quads.push_back(UI::Quad(desc));
        desc.position.y()-=110;
        desc.color=Vec4f(1,1,0,1);
        m_Quads.push_back(UI::Quad(desc));
    }

private:
    Scope<UI::Renderer> m_Renderer;
    std::vector<UI::Quad> m_Quads;

    
};
int main()
{
    WindowContext::Init();
    {
        auto window = std::unique_ptr<Window>(Window::Create(800, 600, "Hello, Aether"));
        // 会在window中创建vulkan对象,在销毁vulkan context前必须调用window 的ReleaseVulkanObjects 销毁window中的vulkan对象
        vk::GRC::Init(window.get(), true);
        auto* testLayer = new TestLayer();
        window->PushLayer(testLayer);
        while (!window->ShouldClose())
        {
            WindowContext::PollEvents();
            window->OnRender();
        }
        vkDeviceWaitIdle(vk::GRC::GetDevice());
        delete testLayer;
        window->ReleaseVulkanObjects();
        vk::GRC::Cleanup();
    }

    WindowContext::Shutdown();
}