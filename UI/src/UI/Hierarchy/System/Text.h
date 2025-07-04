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
    struct FontSignature
    {
        std::string path;
        //如果不需要hinting,worldSize都设置为0，如果需要hinting,worldSize需要是有效的值
        float worldSize=0;
        bool operator==(const FontSignature& other) const
        {
            return path == other.path && worldSize == other.worldSize;
        }
        bool operator!=(const FontSignature& other) const
        {
            return !(*this == other);
        }

    };
    struct FontSignatureHash
    {
        std::size_t operator()(const FontSignature& sig) const
        {
            return std::hash<std::string>()(sig.path) ^ std::hash<float>()(sig.worldSize);
        }
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
            if(!text.visible)
            {
                continue; // skip invisible text
            }
            if(text.content.empty())
            {
                continue; // skip empty text
            }
            // ensure font
            if (!text.font)
            {
                FontSignature sig{
                    .path = text.fontpath.empty() ? m_DefaultFont : text.fontpath,
                    .worldSize = text.hinting? text.worldSize : 0
                };
                auto iter = m_Fonts.find(sig);
                if (iter == m_Fonts.end())
                {
                    bool res = LoadFont(sig);
                    if (!res)
                    {
                        Debug::Log::Error("failed to load font {},{}:{}", text.fontpath, __FILE__ ":", __LINE__);
                        continue;
                    }
                    auto& font = m_Fonts[sig];
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
            uint32_t prevUnicode = 0;
            auto getKerning=[&text](uint32_t prev,uint32_t cur)->long
            {
                auto& face=*text.font->face;
                if(!(face.handle->face_flags & FT_FACE_FLAG_KERNING))
                {
                    return 0; // no kerning
                }
                FT_Vector res;
                FT_Get_Kerning(face.handle,
               FT_Get_Char_Index(face.handle, prev),
               FT_Get_Char_Index(face.handle, cur),
               FT_KERNING_UNSCALED,
               &res);
               return res.x;
            };
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
                //float kerningX=glyph.kerningX;
                float kerningX= getKerning(prevUnicode, unicode) ;
                x+=(glyph.advance+kerningX)*scale;
                if (x > width+base.position.x())
                {
                    x = base.position.x();
                    y += worldSize;
                }
                prevUnicode = unicode;
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
                .color=text.color
            };

            m_Raster->Render(param,
                             *text.renderResource);
        }
    }
    static TextSystem* Create(DeviceRenderPassView renderPass, DeviceDescriptorPool& descriptorPool)
    {
        TextSystem* system = new TextSystem();
        auto raster = Text::Raster::Create(renderPass, true, descriptorPool, true);
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
    bool LoadFont(const FontSignature& signature)
    {
        std::optional<std::string> filepath;
        for (auto& assetDir : m_AssetDirs)
        {
            Filesystem::Path path1(assetDir);
            path1 /= signature.path;
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
        std::optional<Text::Font> fontOpt;
        if(signature.worldSize)
        {
            fontOpt = Text::Font::Create(face.get(),signature.worldSize,true);
        }
        else 
        {
            fontOpt = Text::Font::Create(face.get()); // no hinting
        }
       
        if (!fontOpt)
        {
            return false;
        }
        Font font{
            .face = std::move(face),
            .font = std::make_unique<Text::Font>(std::move(fontOpt.value()))

        };
        m_Fonts[signature] = std::move(font);
        return true;
    }

private:
    std::unordered_map<FontSignature, Font,FontSignatureHash> m_Fonts;
    std::unique_ptr<Text::Raster> m_Raster;
    std::vector<std::string> m_AssetDirs;
    DeviceDescriptorPool* m_DescriptorPool = nullptr; // not own
    Camera2D* m_Camera;                               // not own
    std::optional<Text::Context> m_Context;
    std::string m_DefaultFont="SourceHanSerifCN-Regular-1.otf";
};
} // namespace Aether::UI