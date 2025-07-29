#pragma once
#include <fribidi.h>
#include <harfbuzz/hb.h>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
namespace Aether::Text
{
struct Run
{
    uint32_t start;
    uint32_t length;
    FriBidiParType direction;
    hb_script_t script;
    std::string language;
};
struct Line
{
    std::vector<Run> runs;
};
inline hb_direction_t FribidiDirectionToHbDirection(FriBidiParType direction)
{
    switch (direction)
    {
    case FRIBIDI_PAR_LTR:
        return HB_DIRECTION_LTR;
    case FRIBIDI_PAR_RTL:
        return HB_DIRECTION_RTL;

    default:
        assert(false && "Unsupported direction");
        return HB_DIRECTION_INVALID;
    }
}

inline std::vector<FriBidiStrIndex> CalculateLineVisualMap(const std::basic_string_view<uint32_t>& text)
{
    int nLineSize = text.size();

    uint32_t* pTempLogicalLine = new uint32_t[nLineSize];
    uint32_t* pTempVisualLine = new uint32_t[nLineSize];
    std::vector<FriBidiStrIndex> pTempPositionLogicToVisual = std::vector<FriBidiStrIndex>(nLineSize);
    FriBidiCharType* pTempBidiTypes = new FriBidiCharType[nLineSize];
    FriBidiLevel* pTempEmbeddingLevels = new FriBidiLevel[nLineSize];
    FriBidiJoiningType* pTempJtypes = new FriBidiJoiningType[nLineSize];
    FriBidiArabicProp* pTempArProps = new FriBidiArabicProp[nLineSize];

    for (int i = 0; i < nLineSize; ++i)
    {
        pTempLogicalLine[i] = text[i];
    }

    // Get letter types.
    fribidi_get_bidi_types(pTempLogicalLine, nLineSize, pTempBidiTypes);

    FriBidiParType baseDirection = FRIBIDI_PAR_ON;
    FriBidiLevel resolveParDir = fribidi_get_par_embedding_levels(pTempBidiTypes, nLineSize, &baseDirection, pTempEmbeddingLevels);

    // joine types.
    fribidi_get_joining_types(pTempLogicalLine, nLineSize, pTempJtypes);

    // arabic join.
    memcpy(pTempArProps, pTempJtypes, nLineSize * sizeof(FriBidiJoiningType));
    fribidi_join_arabic(pTempBidiTypes, nLineSize, pTempEmbeddingLevels, pTempArProps);

    // shapes.
    fribidi_shape(FRIBIDI_FLAG_SHAPE_MIRRORING | FRIBIDI_FLAG_SHAPE_ARAB_PRES | FRIBIDI_FLAG_SHAPE_ARAB_LIGA,
                  pTempEmbeddingLevels, nLineSize, pTempArProps, pTempLogicalLine);

    memcpy(pTempVisualLine, pTempLogicalLine, nLineSize * sizeof(uint32_t));
    for (int i = 0; i < nLineSize; i++)
    {
        pTempPositionLogicToVisual[i] = i;
    }

    FriBidiLevel levels = fribidi_reorder_line(FRIBIDI_FLAGS_ARABIC, pTempBidiTypes, nLineSize,
                                               0, baseDirection, pTempEmbeddingLevels, pTempVisualLine, pTempPositionLogicToVisual.data());

    if (pTempJtypes) { delete[] pTempJtypes; }
    if (pTempArProps) { delete[] pTempArProps; }
    if (pTempLogicalLine) { delete[] pTempLogicalLine; }
    if (pTempEmbeddingLevels) { delete[] pTempEmbeddingLevels; }
    if (pTempBidiTypes) { delete[] pTempBidiTypes; }
    if (pTempVisualLine) { delete[] pTempVisualLine; }

    return pTempPositionLogicToVisual;
}

inline std::vector<Run> SplitToRuns(const std::basic_string_view<uint32_t>& text)
{
    std::vector<Run> res;

    size_t length = text.size();

    hb_unicode_funcs_t* ufuncs = hb_unicode_funcs_get_default();

    hb_script_t currentScript = hb_unicode_script(ufuncs, text[0]);
    uint32_t chunkStart = 0;

    for (size_t i = 1; i < length; i++)
    {
        uint32_t currentText = i;
        hb_script_t script = hb_unicode_script(ufuncs, text[currentText]);

        // Skip for HB_SCRIPT_INHERITED, because it can be diacritics.
        if ((script != currentScript && script != HB_SCRIPT_INHERITED))
        {
            Run chunk;
            chunk.start = chunkStart;
            chunk.length = currentText - chunkStart;
            chunk.script = currentScript;
            res.push_back(chunk);

            chunkStart = currentText;
            currentScript = script;
        }
    }

    uint32_t lastSymbol = length - 1;
    if (chunkStart <= lastSymbol)
    {
        Run chunk;
        chunk.start = chunkStart;
        chunk.length = lastSymbol - chunkStart + 1;
        chunk.script = currentScript;
        res.push_back(chunk);
    }

    return res;
}

} // namespace Aether::Text