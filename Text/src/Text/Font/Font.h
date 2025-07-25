#pragma once
#include "Face.h"
#include <Core/Core.h>
#include <Debug/Log.h>
#include <Text/Layout/Run.h>
namespace Aether::Text
{

struct FontCreateInfo
{
    Library* context;
    Face* face;
    float worldSize;
    bool hinting;
    StrokeParam strokeParam;
    bool useStroke;
};
struct FontGlyphCacheKey
{
    uint32_t glyphIndex;
    StrokeParam strokeParam;
    bool useStroke;
    bool operator==(const FontGlyphCacheKey& other) const
    {
        return glyphIndex == other.glyphIndex && strokeParam.radius == other.strokeParam.radius && strokeParam.lineCap == other.strokeParam.lineCap && strokeParam.lineJoin == other.strokeParam.lineJoin && strokeParam.miterLimit == other.strokeParam.miterLimit && useStroke == other.useStroke;
    }
};
} // namespace Aether::Text
namespace Aether
{

template <>
struct Hash<Text::FontGlyphCacheKey>
{
    size_t operator()(const Text::FontGlyphCacheKey& key) const
    {
        size_t hash = 0;
        hash ^= std::hash<uint32_t>()(key.glyphIndex);
        hash ^= Hash<Text::StrokeParam>()(key.strokeParam);
        hash ^= std::hash<bool>()(key.useStroke);
        return hash;
    }
};
} // namespace Aether
namespace Aether::Text
{
struct Font
{
public:
    struct Glyph
    {
        FT_UInt index;
        int32_t bufferIndex;

        int32_t curveCount;

        // Important glyph metrics in font units.
        FT_Pos width, height;
        FT_Pos bearingX;
        FT_Pos bearingY;
        FT_Pos advance;
        FT_Pos kerningX;
    };
#pragma pack(push, 1) // 设置为 1 字节对齐，使其在内存中紧凑排列
    struct BufferGlyph
    {
        uint32_t start, count; // range of bezier curves belonging to this glyph
    };

    struct BufferCurve
    {
        float x0, y0, x1, y1, x2, y2;
        float align[2];
    };
#pragma pack(pop) // 恢复默认对齐

public:
    static std::optional<Font> Create(const FontCreateInfo& info)
    {
        Font font;
        font.context = info.context;
        font.face = info.face;
        font.worldSize = info.worldSize;
        font.hinting = info.hinting;
        if (info.hinting)
        {
            font.loadFlags = FT_LOAD_NO_BITMAP;
            // font.kerningMode = FT_KERNING_DEFAULT;
            font.kerningMode = FT_KERNING_UNSCALED;
            font.emSize = info.worldSize * 64;
            FT_Error error = FT_Set_Pixel_Sizes(info.face->handle, 0, static_cast<FT_UInt>(std::ceil(info.worldSize)));
            if (error)
            {
                // std::cerr << "[font] error while setting pixel size: " << error << std::endl;
                return std::nullopt;
            }
        }
        else
        {
            font.loadFlags = FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
            font.kerningMode = FT_KERNING_UNSCALED;
            font.emSize = info.face->handle->units_per_EM;
        }
        {
            uint32_t charcode = 0;
            FT_UInt glyphIndex = 0;
            FT_Error error = FT_Load_Glyph(info.face->handle, glyphIndex, font.loadFlags);
            if (error)
            {
                // std::cerr << "[font] error while loading undefined glyph: " << error << std::endl;
                return std::nullopt;
            }
            FontGlyphCacheKey key{
                .glyphIndex = 0,
                .strokeParam = info.strokeParam,
                .useStroke = info.useStroke};
            font.BuildGlyph(key, glyphIndex);
        }
        if (!font.CreateDeviceData())
        {
            return std::nullopt;
        }

        return font;
    }
    struct GlyphIndexWithText
    {
        uint32_t glyphIndex;
        std::string text;//cluster utf8 text
    };
    /**
     * @brief means shaping
    */
    std::vector<GlyphIndexWithText> GetGlyphIndexes(const std::string& _text)
    {
        std::vector<GlyphIndexWithText> glyphIndexes;
        U32String u32Text = U32String(_text);
        // split runs
        auto runs=SplitRunsByDirectionAndScript(u32Text);
        // shaping
        //for(auto& run:runs)
        //{
        //    hb_buffer_t* buffer = hb_buffer_create();
        //    // 设置文本范围
        //    hb_buffer_add_utf8(buffer, , -1, run_start, run_length);
        //}
        U32String reorderedText;
        for(auto& run:runs)
        {
            for(size_t i=run.start;i<run.length+run.start;i++)
            {
                reorderedText.Push(u32Text[i]);
            }
        }
        auto text= reorderedText.ToStdString();
        
        
        for (const char* textIt = text.c_str(); *textIt != '\0';)
        {
            uint32_t charcode = decodeCharcode(&textIt);
            FT_UInt glyphIndex = FT_Get_Char_Index(face->handle, charcode);
            uint32_t u8;
            uint8_t len=Unicode2Utf8(charcode,u8);
            std::string glyphText=std::string((const char*)&(u8),(const char*)&(u8)+len);
            glyphIndexes.push_back({glyphIndex,glyphText});
        }
        return glyphIndexes;
    }
    void BuildGlyph(const FontGlyphCacheKey& key, FT_UInt glyphIndex)
    {
        BufferGlyph bufferGlyph;
        bufferGlyph.start = static_cast<int32_t>(bufferCurves.size());

        short start = 0;
        for (int i = 0; i < face->handle->glyph->outline.n_contours; i++)
        {
            // Note: The end indices in face->glyph->outline.contours are inclusive.
            ConvertContour(bufferCurves, &face->handle->glyph->outline,
                           start, face->handle->glyph->outline.contours[i], emSize);
            start = face->handle->glyph->outline.contours[i] + 1;
        }

        bufferGlyph.count = static_cast<int32_t>(bufferCurves.size()) - bufferGlyph.start;

        int32_t bufferIndex = static_cast<int32_t>(bufferGlyphs.size());
        bufferGlyphs.push_back(bufferGlyph);
        Glyph glyph;
        FT_Vector kerning;
        if (FT_Get_Kerning(face->handle, glyphIndex, glyphIndex, kerningMode, &kerning) == 0)
        {
            glyph.kerningX = kerning.x;
        }
        else
        {
            glyph.kerningX = 0;
        }

        glyph.index = glyphIndex;
        glyph.bufferIndex = bufferIndex;
        glyph.curveCount = bufferGlyph.count;
        glyph.width = face->handle->glyph->metrics.width;
        glyph.height = face->handle->glyph->metrics.height;
        glyph.bearingX = (*face).handle->glyph->metrics.horiBearingX;
        glyph.bearingY = (*face).handle->glyph->metrics.horiBearingY;
        glyph.advance = (*face).handle->glyph->metrics.horiAdvance;
        size_t index = bufferGlyphInfo.size();
        bufferGlyphInfo.emplace_back(glyph);
        glyphs[key] = index;
    }
    uint32_t decodeCharcode(const char** text)
    {
        uint8_t first = static_cast<uint8_t>((*text)[0]);

        // Fast-path for ASCII.
        if (first < 128)
        {
            (*text)++;
            return static_cast<uint32_t>(first);
        }

        // This could probably be optimized a bit.
        uint32_t result;
        int size;
        if ((first & 0xE0) == 0xC0)
        { // 110xxxxx
            result = first & 0x1F;
            size = 2;
        }
        else if ((first & 0xF0) == 0xE0)
        { // 1110xxxx
            result = first & 0x0F;
            size = 3;
        }
        else if ((first & 0xF8) == 0xF0)
        { // 11110xxx
            result = first & 0x07;
            size = 4;
        }
        else
        {
            // Invalid encoding.
            (*text)++;
            return 0;
        }

        for (int i = 1; i < size; i++)
        {
            uint8_t value = static_cast<uint8_t>((*text)[i]);
            // Invalid encoding (also catches a null terminator in the middle of a code point).
            if ((value & 0xC0) != 0x80)
            { // 10xxxxxx
                (*text)++;
                return 0;
            }
            result = (result << 6) | (value & 0x3F);
        }

        (*text) += size;
        return result;
    }
    struct BufferGlyphInfoIndexWidthText
    {
        size_t index;
        std::string text;
    };
    std::vector<BufferGlyphInfoIndexWidthText> prepareGlyphsForText(const std::string& text, const StrokeParam& param, bool useStroke)
    {
        std::vector<BufferGlyphInfoIndexWidthText> res;
        bool changed = false;
        auto glyphIndexes = GetGlyphIndexes(text);
        for (auto& [glyphIndex,text] : glyphIndexes)
        {
            FontGlyphCacheKey key;
            key.glyphIndex = glyphIndex;
            key.strokeParam = param;
            key.useStroke = useStroke;
            auto iter=glyphs.find(key);
            if(iter!=glyphs.end())
            {
                res.push_back({iter->second,text});
                continue;
            }
            FT_Error error = FT_Load_Glyph(face->handle, glyphIndex, loadFlags);
            if (error)
            {
                Debug::Log::Debug("[Font] Error while loading glyph for character: {} Error: {},{}:{}", glyphIndex, error, __FILE__, __LINE__);
                continue;
            }
            BuildGlyph(key, glyphIndex);
            res.push_back({bufferGlyphInfo.size() - 1, text});
            changed = true;
        }
        if (changed)
        {
            UpdateDeviceData();
        }
        return res;
    }
    // This function takes a single contour (defined by firstIndex and
    // lastIndex, both inclusive) from outline and converts it into individual
    // quadratic bezier curves, which are added to the curves vector.
    void ConvertContour(std::vector<BufferCurve>& curves, const FT_Outline* outline, short firstIndex, short lastIndex, float emSize)
    {
        // See https://freetype.org/freetype2/docs/glyphs/glyphs-6.html
        // for a detailed description of the outline format.
        //
        // In short, a contour is a list of points describing line segments
        // and quadratic or cubic bezier curves that form a closed shape.
        //
        // TrueType fonts only contain quadratic bezier curves. OpenType fonts
        // may contain outline data in TrueType format or in Compact Font
        // Format, which also allows cubic beziers. However, in FreeType it is
        // (theoretically) possible to mix the two types of bezier curves, so
        // we handle both at the same time.
        //
        // Each point in the contour has a tag specifying its type
        // (FT_CURVE_TAG_ON, FT_CURVE_TAG_CONIC or FT_CURVE_TAG_CUBIC).
        // FT_CURVE_TAG_ON points sit exactly on the outline, whereas the
        // other types are control points for quadratic/conic bezier curves,
        // which in general do not sit exactly on the outline and are also
        // called off points.
        //
        // Some examples of the basic segments:
        // ON - ON ... line segment
        // ON - CONIC - ON ... quadratic bezier curve
        // ON - CUBIC - CUBIC - ON ... cubic bezier curve
        //
        // Cubic bezier curves must always be described by two CUBIC points
        // inbetween two ON points. For the points used in the TrueType format
        // (ON, CONIC) there is a special rule, that two consecutive points of
        // the same type imply a virtual point of the opposite type at their
        // exact midpoint.
        //
        // For example the sequence ON - CONIC - CONIC - ON describes two
        // quadratic bezier curves where the virtual point forms the joining
        // end point of the two curves: ON - CONIC - [ON] - CONIC - ON.
        //
        // Similarly the sequence ON - ON can be thought of as a line segment
        // or a quadratic bezier curve (ON - [CONIC] - ON). Because the
        // virtual point is at the exact middle of the two endpoints, the
        // bezier curve is identical to the line segment.
        //
        // The font shader only supports quadratic bezier curves, so we use
        // this virtual point rule to represent line segments as quadratic
        // bezier curves.
        //
        // Cubic bezier curves are slightly more difficult, since they have a
        // higher degree than the shader supports. Each cubic curve is
        // approximated by two quadratic curves according to the following
        // paper. This preserves C1-continuity (location of and tangents at
        // the end points of the cubic curve) and the paper even proves that
        // splitting at the parametric center minimizes the error due to the
        // degree reduction. One could also analyze the approximation error
        // and split the cubic curve, if the error is too large. However,
        // almost all fonts use "nice" cubic curves, resulting in very small
        // errors already (see also the section on Font Design in the paper).
        //
        // Quadratic Approximation of Cubic Curves
        // Nghia Truong, Cem Yuksel, Larry Seiler
        // https://ttnghia.github.io/pdf/QuadraticApproximation.pdf
        // https://doi.org/10.1145/3406178

        if (firstIndex == lastIndex) return;

        short dIndex = 1;
        if (outline->flags & FT_OUTLINE_REVERSE_FILL)
        {
            short tmpIndex = lastIndex;
            lastIndex = firstIndex;
            firstIndex = tmpIndex;
            dIndex = -1;
        }

        auto convert = [emSize](const FT_Vector& v) {
            return Eigen::Vector2f(
                (float)v.x / emSize,
                (float)v.y / emSize);
        };

        auto makeMidpoint = [](const Eigen::Vector2f& a, const Eigen::Vector2f& b) {
            return 0.5f * (a + b);
        };

        auto makeCurve = [](const Eigen::Vector2f& p0, const Eigen::Vector2f& p1, const Eigen::Vector2f& p2) {
            BufferCurve result;
            result.x0 = p0.x();
            result.y0 = p0.y();
            result.x1 = p1.x();
            result.y1 = p1.y();
            result.x2 = p2.x();
            result.y2 = p2.y();
            return result;
        };

        // Find a point that is on the curve and remove it from the list.
        Eigen::Vector2f first;
        bool firstOnCurve = (outline->tags[firstIndex] & FT_CURVE_TAG_ON);
        if (firstOnCurve)
        {
            first = convert(outline->points[firstIndex]);
            firstIndex += dIndex;
        }
        else
        {
            bool lastOnCurve = (outline->tags[lastIndex] & FT_CURVE_TAG_ON);
            if (lastOnCurve)
            {
                first = convert(outline->points[lastIndex]);
                lastIndex -= dIndex;
            }
            else
            {
                first = makeMidpoint(convert(outline->points[firstIndex]), convert(outline->points[lastIndex]));
                // This is a virtual point, so we don't have to remove it.
            }
        }

        Eigen::Vector2f start = first;
        Eigen::Vector2f control = first;
        Eigen::Vector2f previous = first;
        char previousTag = FT_CURVE_TAG_ON;
        for (short index = firstIndex; index != lastIndex + dIndex; index += dIndex)
        {
            Eigen::Vector2f current = convert(outline->points[index]);
            char currentTag = FT_CURVE_TAG(outline->tags[index]);
            if (currentTag == FT_CURVE_TAG_CUBIC)
            {
                // No-op, wait for more points.
                control = previous;
            }
            else if (currentTag == FT_CURVE_TAG_ON)
            {
                if (previousTag == FT_CURVE_TAG_CUBIC)
                {
                    Eigen::Vector2f& b0 = start;
                    Eigen::Vector2f& b1 = control;
                    Eigen::Vector2f& b2 = previous;
                    Eigen::Vector2f& b3 = current;

                    Eigen::Vector2f c0 = b0 + 0.75f * (b1 - b0);
                    Eigen::Vector2f c1 = b3 + 0.75f * (b2 - b3);

                    Eigen::Vector2f d = makeMidpoint(c0, c1);

                    curves.push_back(makeCurve(b0, c0, d));
                    curves.push_back(makeCurve(d, c1, b3));
                }
                else if (previousTag == FT_CURVE_TAG_ON)
                {
                    // Linear segment.
                    curves.push_back(makeCurve(previous, makeMidpoint(previous, current), current));
                }
                else
                {
                    // Regular bezier curve.
                    curves.push_back(makeCurve(start, previous, current));
                }
                start = current;
                control = current;
            }
            else /* currentTag == FT_CURVE_TAG_CONIC */
            {
                if (previousTag == FT_CURVE_TAG_ON)
                {
                    // No-op, wait for third point.
                }
                else
                {
                    // Create virtual on point.
                    Eigen::Vector2f mid = makeMidpoint(previous, current);
                    curves.push_back(makeCurve(start, previous, mid));
                    start = mid;
                    control = mid;
                }
            }
            previous = current;
            previousTag = currentTag;
        }

        // Close the contour.
        if (previousTag == FT_CURVE_TAG_CUBIC)
        {
            Eigen::Vector2f& b0 = start;
            Eigen::Vector2f& b1 = control;
            Eigen::Vector2f& b2 = previous;
            Eigen::Vector2f& b3 = first;

            Eigen::Vector2f c0 = b0 + 0.75f * (b1 - b0);
            Eigen::Vector2f c1 = b3 + 0.75f * (b2 - b3);

            Eigen::Vector2f d = makeMidpoint(c0, c1);

            curves.push_back(makeCurve(b0, c0, d));
            curves.push_back(makeCurve(d, c1, b3));
        }
        else if (previousTag == FT_CURVE_TAG_ON)
        {
            // Linear segment.
            curves.push_back(makeCurve(previous, makeMidpoint(previous, first), first));
        }
        else
        {
            curves.push_back(makeCurve(start, previous, first));
        }
    }
    std::string DebugExportBufferJson()
    {
        using Json = nlohmann::json;
        Json json;
        auto curves = Json::array();
        for (auto& curve : bufferCurves)
        {
            Json curveJson;
            curveJson["x0"] = curve.x0;
            curveJson["y0"] = curve.y0;
            curveJson["x1"] = curve.x1;
            curveJson["y1"] = curve.y1;
            curveJson["x2"] = curve.x2;
            curveJson["y2"] = curve.y2;
            curves.push_back(curveJson);
        }
        json["curves"] = curves;
        auto glyphs = Json::array();
        for (auto& glyph : bufferGlyphs)
        {
            Json glyphJson;
            glyphJson["start"] = glyph.start;
            glyphJson["count"] = glyph.count;
            glyphs.push_back(glyphJson);
        }
        json["glyphs"] = glyphs;
        return json.dump(4);
    }
    bool UpdateDeviceData();
    bool CreateDeviceData(); // texture buffer sampler

public:
    Face* face;                                                                    // not own
    std::unordered_map<FontGlyphCacheKey, size_t, Hash<FontGlyphCacheKey>> glyphs; // glyph index and stroke param -> buffer glyph index
    std::vector<Glyph> bufferGlyphInfo;
    bool hinting;
    FT_Int32 loadFlags;
    FT_Kerning_Mode kerningMode;
    std::vector<BufferGlyph> bufferGlyphs;
    std::vector<BufferCurve> bufferCurves;
    float emSize;
    float worldSize; // world -> screen(or framebuffer) , worldSize-> pixelSize
    DeviceBuffer stagingBuffer;
    DeviceTexture glyphTexture;
    DeviceTexture curveTexture;
    Library* context; // not own
    StrokeParam strokeParam;
    bool useStroke = false;
};
} // namespace Aether::Text
