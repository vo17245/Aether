#pragma once
#include <fribidi.h>
#include <harfbuzz/hb.h>
#include <vector>
#include <string>
namespace Aether::Text
{
struct Run
{
    uint32_t start;
    uint32_t length;
    FriBidiParType direction;
    hb_script_t script;
    std::string language="en";
};
inline std::vector<Run> SplitRunsByDirectionAndScript(const std::basic_string_view<uint32_t>& text32)
{
    int len = text32.size();
    std::vector<Run> runs;

    // 1. 使用 FriBidi 获取方向信息
    std::vector<FriBidiChar> fribidi_input(text32.begin(), text32.end());
    std::vector<FriBidiParType> bidi_types(len);
    std::vector<FriBidiLevel> bidi_levels(len);
    FriBidiParType base_dir = FRIBIDI_PAR_ON;

    fribidi_get_bidi_types(fribidi_input.data(), len, bidi_types.data());
    fribidi_get_par_embedding_levels(bidi_types.data(), len, &base_dir, bidi_levels.data());

    // 2. 使用 HarfBuzz 获取 Script 信息
    std::vector<hb_script_t> scripts(len);
    for (int i = 0; i < len; ++i)
    {
        scripts[i] = hb_unicode_script(hb_unicode_funcs_get_default(), text32[i]);
    }

    // 3. 合并方向和脚本来分割 run
    uint32_t run_start = 0;
    FriBidiParType cur_dir = base_dir;
    hb_script_t cur_script = scripts[0];

    for (int i = 1; i < len; ++i)
    {
        if (bidi_levels[i] != bidi_levels[run_start] || scripts[i] != cur_script)
        {
            runs.push_back({run_start, i - run_start, bidi_levels[run_start] & 1 ? (FriBidiParType)FRIBIDI_PAR_RTL : (FriBidiParType)FRIBIDI_PAR_LTR, cur_script});
            run_start = i;
            cur_script = scripts[i];
        }
    }
    // push last run
    uint32_t run_length = len - run_start;
    if (run_length > 0)
    {
        runs.push_back({run_start, run_length, bidi_levels[run_start] & 1 ? (FriBidiParType)FRIBIDI_PAR_RTL : (FriBidiParType)FRIBIDI_PAR_LTR, cur_script});
    }

    return runs;
}
} // namespace Aether::Text