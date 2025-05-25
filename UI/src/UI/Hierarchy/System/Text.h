#pragma once
#include "System.h"
#include "Text/Font.h"
#include "Text/Raster/Raster.h"
#include "UI/Hierarchy/Component/Base.h"
#include <memory>
#include "UI/Hierarchy/Component/Text.h"
#include <unordered_map>
#include <Filesystem/FilesystemApi.h>
#include <Debug/Log.h>
namespace Aether::UI
{
class TextSystem : public SystemI
{
public:
    struct Font
    {
        // 避免移动导致指针失效，创建在堆上
        std::unique_ptr<Text::Face> face;
        std::unique_ptr<Text::Font> font;
    };
public:
    virtual void OnUpdate(float sec, Scene& scene)
    {
    }
    virtual void OnRender(DeviceCommandBufferView commandBuffer,
                          DeviceRenderPassView renderPass,
                          DeviceFrameBufferView frameBuffer,
                          Vec2f screenSize,
                          Scene& scene)
    {
        
        auto view=scene.Select<BaseComponent,TextComponent>();
        
        for(const auto&[entity,base,text]:view.each())
        {
            // ensure font
            if(!text.font)
            {
                auto iter=m_Fonts.find(text.fontpath);
                if(iter==m_Fonts.end())
                {
                    bool res=LoadFont(text.fontpath);
                    if(!res)
                    {
                        Debug::Log::Error("failed to load font {},{}:{}",text.fontpath,__FILE__":",__LINE__);
                        continue;
                    }
                    auto& font=m_Fonts[text.fontpath];
                    text.font=font.font.get();
                }
            }
            // calculate glyph position
            float width=base.size.x();
            float height=base.size.y();
            float x=base.position.x();
            float y=base.position.y();
            // render glyph
        }
    }
    static TextSystem* Create(DeviceRenderPassView renderPass,DeviceDescriptorPool &descriptorPool, const Vec2f &screenSize)
    {
        TextSystem* system=new TextSystem();
        auto raster=Text::Raster::Create(renderPass, true, descriptorPool, screenSize);
        if(!raster)
        {
            return nullptr;
        }
        system->m_Raster=std::make_unique<Text::Raster>(std::move(raster.value()));
        auto contextOpt=Text::Context::Create();
        if(!contextOpt)
        {
            assert(false&&"failed to create text context(freetype library)");
            return nullptr;
        }
        system->m_Context=std::move(Text::Context::Create().value());
        return system;
    }

    void AddAssetDir(const std::string& path)
    {
        m_AssetDirs.emplace_back(path);
    }

private:
    TextSystem()=default;
    bool LoadFont(const std::string& path)
    {
        std::optional<std::string> filepath;
        for(auto& assetDir:m_AssetDirs)
        {
            Filesystem::Path path(assetDir);
            path/=path;
            if(Filesystem::Exists(path))
            {
                filepath=path;
                break;
            }
        }
        if(!filepath)
        {
            return false;
        }
        auto faceOpt=Text::Face::Create(*m_Context, filepath->c_str());

        if(!faceOpt)
        {
            return false;
        }
        auto face=std::make_unique<Text::Face>(std::move(faceOpt.value()));
        auto fontOpt=Text::Font::Create(face.get());
        if(!fontOpt)
        {
            return false;
        }
        Font font{
            .face=std::move(face),
            .font=std::make_unique<Text::Font>(std::move(fontOpt.value()))
            
        };
        m_Fonts[path]=std::move(font);
        return true;
    }
private:
    
    std::unordered_map<std::string,Font> m_Fonts;
    std::unique_ptr<Text::Raster> m_Raster;
    std::vector<std::string> m_AssetDirs;
    
    std::optional<Text::Context> m_Context;
    
    
};
} // namespace Aether::UI