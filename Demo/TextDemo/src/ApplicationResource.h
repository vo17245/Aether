#pragma once
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceRenderPass.h"
#include "Render/RenderApi/DeviceTexture.h"
#include "Render/Scene/Camera2D.h"
#include "Render/Vulkan/RenderPass.h"
#include "Text/Font.h"
#include "Text/Raster/Raster.h"
using namespace Aether;
class ApplicationResource
{
public:
    static std::optional<std::string> Init(const Vec2f& screenSize)
    {
        s_Instance = new ApplicationResource();

        return s_Instance->InitImpl(screenSize);

    }
    static void Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
    ~ApplicationResource()
    {
       
    }
    static ApplicationResource& GetSingleton()
    {
        return *s_Instance;
    }
    Scope<Text::Raster> textRaster;
    DeviceRenderPass defaultRenderPass;
    DeviceDescriptorPool descriptorPool;
    Scope<Text::Font> font;
    Scope<Text::Context> textContext;
    Scope<Text::Face> textFace;
    Scope<Text::Raster::RenderPassResource> rasterResource;
    Camera2D camera;
private:
    void InitCamera(const Vec2f& screenSize)
    {
        camera.screenSize = screenSize;
        camera.target = Vec2f(screenSize.x() / 2, screenSize.y() / 2);
        camera.offset = Vec2f(0, 0);
        camera.near = 0.0f;
        camera.far = 10000.0f;
        camera.zoom = 1.0f;
        camera.rotation = 0.0f;
    }
    std::optional<std::string> InitImpl(const Vec2f& screenSize)
    {
        defaultRenderPass=vk::RenderPass::CreateDefault().value();
        descriptorPool=DeviceDescriptorPool::Create().value();
        auto textRasterOpt=Text::Raster::Create(defaultRenderPass, 
            true, descriptorPool,false,
        PackFlags(Text::Raster::Keyword::Fill));
        textRaster=CreateScope<Text::Raster>(std::move(textRasterOpt.value()));
        auto textContextOpt=Text::Context::Create();
        textContext=CreateScope<Text::Context>(std::move(textContextOpt.value()));
        auto textFaceOpt=Text::Face::Create(*textContext, "Assets/SourceHanSerifCN-Regular-1.otf");
        textFace=CreateScope<Text::Face>(std::move(textFaceOpt.value()));
        auto fontOpt=Text::Font::Create(textFace.get());
        font=CreateScope<Text::Font>(std::move(fontOpt.value()));
        auto resource=textRaster->CreateRenderPassResource();
        rasterResource=CreateScope<Text::Raster::RenderPassResource>(std::move(resource));
        InitCamera(screenSize);
        return std::nullopt;
    }
    static ApplicationResource* s_Instance;
    
};
