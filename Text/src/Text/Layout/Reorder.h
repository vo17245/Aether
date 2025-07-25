#pragma once
#include <fribidi.h>
#include <Core/Core.h>
namespace Aether::Text
{
struct UnicodeBidiMap
{
    std::vector<FriBidiStrIndex> L2V;
    std::vector<FriBidiStrIndex> V2L;
    // 0,2,4... even is LTR 
    // 1,3,5... odd is RTL
    std::vector<FriBidiLevel> levels;
    FriBidiParType baseDirection;
};
inline UnicodeBidiMap CalculateUnicodeBidiMap(const std::basic_string_view<uint32_t>& text)
{
    UnicodeBidiMap bidiMap;

    size_t len = text.size();
    // Allocate output buffers
    std::vector<FriBidiChar> visual_str(len);
    std::vector<FriBidiStrIndex> position_L_to_V_list(len);
    std::vector<FriBidiStrIndex> position_V_to_L_list(len);
    std::vector<FriBidiLevel> embedding_level_list(len);

    // Set paragraph direction (auto-detect)
    FriBidiParType base_dir = FRIBIDI_PAR_ON;

    // Call fribidi_log2vis
    FriBidiLevel max_level = fribidi_log2vis(
        /* input */
        text.data(),
        len,
        &base_dir,
        /* output */
        visual_str.data(),
        position_L_to_V_list.data(),
        position_V_to_L_list.data(),
        embedding_level_list.data());
    bidiMap.baseDirection = base_dir;
    bidiMap.L2V = std::move(position_L_to_V_list);
    bidiMap.V2L = std::move(position_V_to_L_list);
    bidiMap.levels = std::move(embedding_level_list);
    return bidiMap;
}

} // namespace Aether::Text