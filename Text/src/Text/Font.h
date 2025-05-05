#pragma once
#include <freetype/freetype.h>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <Core/LocalRef.h>
#include <optional>
#include <Eigen/Core>
#include <nlohmann/json.hpp>
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
    };
    struct BufferGlyph
    {
        int32_t start, count; // range of bezier curves belonging to this glyph
    };

    struct BufferCurve
    {
        float x0, y0, x1, y1, x2, y2;
    };

    struct BufferVertex
    {
        float x, y, u, v;
        int32_t bufferIndex;
    };

public:
    static std::optional<Font> Create(LocalRef<FT_Face> face, float worldSize = 32.0f, bool hinting = false)
    {
        Font font;
        font.face=face;
        font.worldSize = worldSize;
        font.hinting = hinting;
        if (hinting) {
			font.loadFlags = FT_LOAD_NO_BITMAP;
			font.kerningMode = FT_KERNING_DEFAULT;

			font.emSize = worldSize * 64;
			FT_Error error = FT_Set_Pixel_Sizes(*face, 0, static_cast<FT_UInt>(std::ceil(worldSize)));
			if (error) {
				//std::cerr << "[font] error while setting pixel size: " << error << std::endl;
                return std::nullopt;
			}

		} else {
			font.loadFlags = FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
			font.kerningMode = FT_KERNING_UNSCALED;
			font.emSize = (*face)->units_per_EM;
		}
        {
			uint32_t charcode = 0;
			FT_UInt glyphIndex = 0;
			FT_Error error = FT_Load_Glyph(*face, glyphIndex, font.loadFlags);
			if (error) {
				//std::cerr << "[font] error while loading undefined glyph: " << error << std::endl;
                return std::nullopt;
			}
			font.buildGlyph(charcode, glyphIndex);
		}
        return font;
    }
    void buildGlyph(uint32_t charcode, FT_UInt glyphIndex)
    {
        BufferGlyph bufferGlyph;
		bufferGlyph.start = static_cast<int32_t>(bufferCurves.size());

		short start = 0;
		for (int i = 0; i < (*face)->glyph->outline.n_contours; i++) {
			// Note: The end indices in face->glyph->outline.contours are inclusive.
			convertContour(bufferCurves, &(*face)->glyph->outline, start, (*face)->glyph->outline.contours[i], emSize);
			start = (*face)->glyph->outline.contours[i]+1;
		}

		bufferGlyph.count = static_cast<int32_t>(bufferCurves.size()) - bufferGlyph.start;

		int32_t bufferIndex = static_cast<int32_t>(bufferGlyphs.size());
		bufferGlyphs.push_back(bufferGlyph);

		Glyph glyph;
		glyph.index = glyphIndex;
		glyph.bufferIndex = bufferIndex;
		glyph.curveCount = bufferGlyph.count;
		glyph.width = (*face)->glyph->metrics.width;
		glyph.height = (*face)->glyph->metrics.height;
		glyph.bearingX = (*face)->glyph->metrics.horiBearingX;
		glyph.bearingY = (*face)->glyph->metrics.horiBearingY;
		glyph.advance = (*face)->glyph->metrics.horiAdvance;
		glyphs[charcode] = glyph;
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
    void prepareGlyphsForText(const std::string& text)
    {
        bool changed=false;
        for (const char* textIt = text.c_str(); *textIt != '\0';)
        {
            uint32_t charcode = decodeCharcode(&textIt);

            if (charcode == '\r' || charcode == '\n') continue;
            if (glyphs.count(charcode) != 0) continue;

            FT_UInt glyphIndex = FT_Get_Char_Index(*face, charcode);
            if (!glyphIndex) continue;

            FT_Error error = FT_Load_Glyph(*face, glyphIndex, loadFlags);
            if (error)
            {
                //std::cerr << "[font] error while loading glyph for character " << charcode << ": " << error << std::endl;
                continue;
            }

            buildGlyph(charcode, glyphIndex);
            changed = true;
        }
    }
    // This function takes a single contour (defined by firstIndex and
	// lastIndex, both inclusive) from outline and converts it into individual
	// quadratic bezier curves, which are added to the curves vector.
	void convertContour(std::vector<BufferCurve>& curves, const FT_Outline* outline, short firstIndex, short lastIndex, float emSize) {
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
		if (outline->flags & FT_OUTLINE_REVERSE_FILL) {
			short tmpIndex = lastIndex;
			lastIndex = firstIndex;
			firstIndex = tmpIndex;
			dIndex = -1;
		}

		auto convert = [emSize](const FT_Vector& v) {
			return Eigen::Vector2f(
				(float)v.x / emSize,
				(float)v.y / emSize
			);
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
		if (firstOnCurve) {
			first = convert(outline->points[firstIndex]);
			firstIndex += dIndex;
		} else {
			bool lastOnCurve = (outline->tags[lastIndex] & FT_CURVE_TAG_ON);
			if (lastOnCurve) {
				first = convert(outline->points[lastIndex]);
				lastIndex -= dIndex;
			} else {
				first = makeMidpoint(convert(outline->points[firstIndex]), convert(outline->points[lastIndex]));
				// This is a virtual point, so we don't have to remove it.
			}
		}

		Eigen::Vector2f start = first;
		Eigen::Vector2f control = first;
		Eigen::Vector2f previous = first;
		char previousTag = FT_CURVE_TAG_ON;
		for (short index = firstIndex; index != lastIndex + dIndex; index += dIndex) {
			Eigen::Vector2f current = convert(outline->points[index]);
			char currentTag = FT_CURVE_TAG(outline->tags[index]);
			if (currentTag == FT_CURVE_TAG_CUBIC) {
				// No-op, wait for more points.
				control = previous;
			} else if (currentTag == FT_CURVE_TAG_ON) {
				if (previousTag == FT_CURVE_TAG_CUBIC) {
					Eigen::Vector2f& b0 = start;
					Eigen::Vector2f& b1 = control;
					Eigen::Vector2f& b2 = previous;
					Eigen::Vector2f& b3 = current;

					Eigen::Vector2f c0 = b0 + 0.75f * (b1 - b0);
					Eigen::Vector2f c1 = b3 + 0.75f * (b2 - b3);

					Eigen::Vector2f d = makeMidpoint(c0, c1);

					curves.push_back(makeCurve(b0, c0, d));
					curves.push_back(makeCurve(d, c1, b3));
				} else if (previousTag == FT_CURVE_TAG_ON) {
					// Linear segment.
					curves.push_back(makeCurve(previous, makeMidpoint(previous, current), current));
				} else {
					// Regular bezier curve.
					curves.push_back(makeCurve(start, previous, current));
				}
				start = current;
				control = current;
			} else /* currentTag == FT_CURVE_TAG_CONIC */ {
				if (previousTag == FT_CURVE_TAG_ON) {
					// No-op, wait for third point.
				} else {
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
		if (previousTag == FT_CURVE_TAG_CUBIC) {
			Eigen::Vector2f& b0 = start;
			Eigen::Vector2f& b1 = control;
			Eigen::Vector2f& b2 = previous;
			Eigen::Vector2f& b3 = first;

			Eigen::Vector2f c0 = b0 + 0.75f * (b1 - b0);
			Eigen::Vector2f c1 = b3 + 0.75f * (b2 - b3);

			Eigen::Vector2f d = makeMidpoint(c0, c1);

			curves.push_back(makeCurve(b0, c0, d));
			curves.push_back(makeCurve(d, c1, b3));

		} else if (previousTag == FT_CURVE_TAG_ON) {
			// Linear segment.
			curves.push_back(makeCurve(previous, makeMidpoint(previous, first), first));
		} else {
			curves.push_back(makeCurve(start, previous, first));
		}
	}
	std::string DebugExportBufferJson()
	{
		using Json=nlohmann::json;
		Json json;
		auto curves=Json::array();
		for (auto& curve : bufferCurves) {
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
		for (auto& glyph : bufferGlyphs) {
			Json glyphJson;
			glyphJson["start"] = glyph.start;
			glyphJson["count"] = glyph.count;
			glyphs.push_back(glyphJson);
		}
		json["glyphs"] = glyphs;
		return json.dump(4);
		
	}


public:
    LocalRef<FT_Face> face;
    std::unordered_map<uint32_t, Glyph> glyphs;
    bool hinting;
    FT_Int32 loadFlags;
    FT_Kerning_Mode kerningMode;
    std::vector<BufferGlyph> bufferGlyphs;
	std::vector<BufferCurve> bufferCurves;
    float emSize;
    float worldSize; // world -> screen(or framebuffer) , worldSize-> pixelSize
};
} // namespace AeExporter::ContourRenderer