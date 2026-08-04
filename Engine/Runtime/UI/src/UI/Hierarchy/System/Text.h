#pragma once
#include <Render/RenderGraph/RenderGraph.h>
#include "Render/Scene/Camera2D.h"
#include "System.h"
#include "Text/Font/Font.h"
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
        // 如果不需要hinting,worldSize都设置为0，如果需要hinting,worldSize需要是有效的值
        float worldSize = 0;
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
    virtual void OnUpdate(float sec, World& scene)
    {
    }
    virtual void OnBuildRenderGraph(RenderGraph::RenderGraph& renderGraph,
                                    const RenderGraph::RenderPassDesc& renderPassDesc,
                                    Vec2f screenSize,
                                    World& scene) override
    {
        (void)screenSize;
        auto view = scene.Select<BaseComponent, TextComponent>();

        for (const auto& [entity, base, text] : view.each())
        {
            (void)entity;
            if (!text.visible || text.content.empty())
            {
                continue;
            }
            if (!text.font)
            {
                FontSignature sig{
                    .path = text.fontpath.empty() ? m_DefaultFont : text.fontpath,
                    .worldSize = text.hinting ? text.worldSize : 0};
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
                    text.font = iter->second.font.get();
                }
            }

            auto lines = text.font->prepareGlyphsForText(text.content, {});
            if (!text.renderResource)
            {
                auto resource = m_Raster->CreateRenderPassResource();
                text.renderResource = std::make_unique<decltype(resource)>(std::move(resource));
            }

            float width = base.size.x();
            float x = base.position.x();
            float y = base.position.y();
            std::vector<Vec2f> glyphPos;
            std::vector<uint32_t> glyphToRender;
            float worldSize = text.worldSize;
            for (auto& line : lines)
            {
                for (auto& glyph : line.visualGlyphs)
                {
                    auto& info = text.font->bufferGlyphInfo[glyph.indexInBuffer];
                    float emSize = info.emSize;
                    float scale = worldSize / info.emSize;
                    float curY = y + (emSize - info.bearingY) * scale;
                    glyphToRender.push_back(glyph.indexInBuffer);
                    glyphPos.emplace_back(Vec2f(x, curY));
                    x += info.advance * scale;
                    if (x > width + base.position.x())
                    {
                        x = base.position.x();
                        y += worldSize;
                    }
                }
                x = base.position.x();
                y += worldSize;
            }
            if (glyphToRender.empty())
            {
                continue;
            }
            Text::Raster::RenderPassParam param{
                .renderGraph = &renderGraph,
                .renderPassDesc = renderPassDesc,
                .font = *text.font,
                .bufferGlyphInfoIndexes = glyphToRender,
                .glyphPosition = glyphPos,
                .worldSize = worldSize,
                .camera = *m_Camera,
                .z = base.z,
                .color = text.color};

            m_Raster->Render(param, *text.renderResource);
        }
    }
    static TextSystem* Create()
    {
        TextSystem* system = new TextSystem();
        auto raster = Text::Raster::Create(true, true);
        if (!raster)
        {
            return nullptr;
        }
        system->m_Raster = std::make_unique<Text::Raster>(std::move(raster.value()));
        auto contextOpt = Text::Library::Create();
        if (!contextOpt)
        {
            assert(false && "failed to create text context(freetype library)");
            return nullptr;
        }
        system->m_Context = CreateScope<Text::Library>(std::move(Text::Library::Create().value()));
        return system;
    }

    void AddAssetDir(const std::string& path)
    {
        m_AssetDirs.emplace_back(path);
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
        auto fontOpt = Text::Font::Create(m_Context.get(),face.get());
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
    std::unordered_map<FontSignature, Font, FontSignatureHash> m_Fonts;
    std::unique_ptr<Text::Raster> m_Raster;
    std::vector<std::string> m_AssetDirs;
    Camera2D* m_Camera = nullptr;                     // not own
    Scope<Text::Library> m_Context;
    std::string m_DefaultFont = "SourceHanSerifCN-Regular-1.otf";
};
} // namespace Aether::UI