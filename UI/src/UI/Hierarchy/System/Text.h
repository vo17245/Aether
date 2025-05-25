#pragma once
#include "Render/RenderApi/DeviceDescriptorPool.h"
#include "Render/Scene/Camera2D.h"
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
    ~TextSystem()

    {
        m_Fonts.clear();
    }

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
        assert(m_DescriptorPool && "descriptor pool is null");
        auto view = scene.Select<BaseComponent, TextComponent>();

        for (const auto& [entity, base, text] : view.each())
        {
            // ensure font
            if (!text.font)
            {
                auto& fontPath=text.fontpath.empty()?m_DefaultFont:text.fontpath;
                auto iter = m_Fonts.find(fontPath);
                if (iter == m_Fonts.end())
                {
                    bool res = LoadFont(fontPath);
                    if (!res)
                    {
                        Debug::Log::Error("failed to load font {},{}:{}", text.fontpath, __FILE__ ":", __LINE__);
                        continue;
                    }
                    auto& font = m_Fonts[fontPath];
                    text.font = font.font.get();
                }
                else
                {
                    text.font=iter->second.font.get();
                }
            }
            text.font->prepareGlyphsForText(text.content);
            // ensure render resource
            if (!text.renderResource)
            {
                auto resource = m_Raster->CreateRenderPassResource();
                text.renderResource = std::make_unique<decltype(resource)>(std::move(resource));
            }
            // calculate glyph position
            U32String u32s(text.content);
            float width = base.size.x();
            float height = base.size.y();
            float x = base.position.x();
            float y = base.position.y();
            std::vector<Vec2f> glyphPos;
            glyphPos.reserve(u32s.Size());
            float worldSize = text.worldSize;
            float scale = worldSize / text.font->emSize;
            for (auto unicode : u32s.GetData())
            {
                if(unicode=='\n')
                {
                    x = base.position.x();
                    y += worldSize;
                    glyphPos.emplace_back(Vec2f(0, 0));
                    continue;
                }
                auto& glyph = text.font->glyphs[unicode];
                float curY = y + (text.font->emSize - glyph.bearingY) * scale;
                glyphPos.emplace_back(Vec2f(x, curY));
                x+=(glyph.advance+glyph.kerningX)*scale;
                if (x > width+base.position.x())
                {
                    x = base.position.x();
                    y += worldSize;
                }
            }
            // render glyph
            Text::Raster::RenderPassParam param{
                .commandBuffer = commandBuffer,
                .renderPass = renderPass,
                .frameBuffer = frameBuffer,
                .descriptorPool = *m_DescriptorPool,
                .font = *text.font,
                .unicodes = u32s.GetData(),
                .glyphPosition = glyphPos,
                .worldSize = worldSize,
                .camera = *m_Camera,
                .z = base.z,
            };

            m_Raster->Render(param,
                             *text.renderResource);
        }
    }
    static TextSystem* Create(DeviceRenderPassView renderPass, DeviceDescriptorPool& descriptorPool, const Vec2f& screenSize)
    {
        TextSystem* system = new TextSystem();
        auto raster = Text::Raster::Create(renderPass, true, descriptorPool, screenSize);
        if (!raster)
        {
            return nullptr;
        }
        system->m_Raster = std::make_unique<Text::Raster>(std::move(raster.value()));
        auto contextOpt = Text::Context::Create();
        if (!contextOpt)
        {
            assert(false && "failed to create text context(freetype library)");
            return nullptr;
        }
        system->m_Context = std::move(Text::Context::Create().value());
        return system;
    }

    void AddAssetDir(const std::string& path)
    {
        m_AssetDirs.emplace_back(path);
    }
    void SetDescriptorPool(DeviceDescriptorPool* pool)
    {
        m_DescriptorPool = pool;
    }
    void SetCamera(Camera2D* camera)
    {
        m_Camera = camera;
    }

private:
    TextSystem() = default;
    bool LoadFont(const std::string& path)
    {
        std::optional<std::string> filepath;
        for (auto& assetDir : m_AssetDirs)
        {
            Filesystem::Path path1(assetDir);
            path1 /= path;
            if (Filesystem::Exists(path1))
            {
                filepath = path1;
                break;
            }
        }
        if (!filepath)
        {
            return false;
        }
        auto faceOpt = Text::Face::Create(*m_Context, filepath->c_str());

        if (!faceOpt)
        {
            return false;
        }
        auto face = std::make_unique<Text::Face>(std::move(faceOpt.value()));
        auto fontOpt = Text::Font::Create(face.get());
        if (!fontOpt)
        {
            return false;
        }
        Font font{
            .face = std::move(face),
            .font = std::make_unique<Text::Font>(std::move(fontOpt.value()))

        };
        m_Fonts[path] = std::move(font);
        return true;
    }

private:
    std::unordered_map<std::string, Font> m_Fonts;
    std::unique_ptr<Text::Raster> m_Raster;
    std::vector<std::string> m_AssetDirs;
    DeviceDescriptorPool* m_DescriptorPool = nullptr; // not own
    Camera2D* m_Camera;                               // not own
    std::optional<Text::Context> m_Context;
    std::string m_DefaultFont="SourceHanSerifCN-Regular-1.otf";
};
} // namespace Aether::UI