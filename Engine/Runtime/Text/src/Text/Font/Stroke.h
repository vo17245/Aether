#pragma once
#include "Filesystem/Utils.h"
#include <freetype/freetype.h>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <Core/Base.h>
#include <optional>
#include <Eigen/Core>
#include <nlohmann/json.hpp>
#include <Render/RenderApi.h>
#include <ft2build.h>
#include FT_STROKER_H
#include <Core/Core.h>
namespace Aether::Text
{
    /**
 *  @values:
 *   FT_STROKER_LINECAP_BUTT ::
 *     The end of lines is rendered as a full stop on the last point
 *     itself.
 *
 *   FT_STROKER_LINECAP_ROUND ::
 *     The end of lines is rendered as a half-circle around the last point.
 *
 *   FT_STROKER_LINECAP_SQUARE ::
 *     The end of lines is rendered as a square around the last point.
 */
enum class LineCap : uint32_t
{
    Butt = FT_STROKER_LINECAP_BUTT,     // 平头线帽
    Round = FT_STROKER_LINECAP_ROUND,   // 圆形线帽
    Square = FT_STROKER_LINECAP_SQUARE, // 方形线帽
};
/**
 *
 *
 * @values:
 *   FT_STROKER_LINEJOIN_ROUND ::
 *     Used to render rounded line joins.  Circular arcs are used to join
 *     two lines smoothly.
 *
 *   FT_STROKER_LINEJOIN_BEVEL ::
 *     Used to render beveled line joins.  The outer corner of the joined
 *     lines is filled by enclosing the triangular region of the corner
 *     with a straight line between the outer corners of each stroke.
 *
 *   FT_STROKER_LINEJOIN_MITER_FIXED ::
 *     Used to render mitered line joins, with fixed bevels if the miter
 *     limit is exceeded.  The outer edges of the strokes for the two
 *     segments are extended until they meet at an angle.  A bevel join
 *     (see above) is used if the segments meet at too sharp an angle and
 *     the outer edges meet beyond a distance corresponding to the meter
 *     limit.  This prevents long spikes being created.
 *     `FT_STROKER_LINEJOIN_MITER_FIXED` generates a miter line join as
 *     used in PostScript and PDF.
 *
 *   FT_STROKER_LINEJOIN_MITER_VARIABLE ::
 *   FT_STROKER_LINEJOIN_MITER ::
 *     Used to render mitered line joins, with variable bevels if the miter
 *     limit is exceeded.  The intersection of the strokes is clipped
 *     perpendicularly to the bisector, at a distance corresponding to
 *     the miter limit. This prevents long spikes being created.
 *     `FT_STROKER_LINEJOIN_MITER_VARIABLE` generates a mitered line join
 *     as used in XPS.  `FT_STROKER_LINEJOIN_MITER` is an alias for
 *     `FT_STROKER_LINEJOIN_MITER_VARIABLE`, retained for backward
 *     compatibility.
 */
enum class LineJoin : uint32_t
{

    Round = FT_STROKER_LINEJOIN_ROUND,
    Bevel = FT_STROKER_LINEJOIN_BEVEL,
    Variable = FT_STROKER_LINEJOIN_MITER_VARIABLE,
    Miter = FT_STROKER_LINEJOIN_MITER,
    MiterFixed = FT_STROKER_LINEJOIN_MITER_FIXED,

};

struct StrokeParam
{
    uint32_t radius = 0;
    LineCap lineCap = LineCap::Butt;
    LineJoin lineJoin = LineJoin::Round;
    uint32_t miterLimit = 32;
	bool operator==(const StrokeParam& other) const
	{
		return radius == other.radius && lineCap == other.lineCap &&
			lineJoin == other.lineJoin && miterLimit == other.miterLimit ;
	}
};


}
namespace Aether
{
    template<>
    struct Hash<Text::StrokeParam>
    {
        bool operator()(const Text::StrokeParam& param) const
        {
            size_t hash = 0;
            hash ^= std::hash<float>()(param.radius);
            hash ^= std::hash<int>()(static_cast<int>(param.lineCap));
            hash ^= std::hash<int>()(static_cast<int>(param.lineJoin));
            hash ^= std::hash<float>()(param.miterLimit);
            return hash;
        }
    };
}