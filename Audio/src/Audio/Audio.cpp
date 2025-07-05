#include "Audio.h"
#include <miniaudio/miniaudio.h>
namespace Aether::Audio
{
ma_engine g_Engine;

int Init()
{
    return ma_engine_init(NULL, &g_Engine);
}
void Destory()
{
    ma_engine_uninit(&g_Engine);
}

int Play(const char* path)
{
    return ma_engine_play_sound(&g_Engine, path, NULL);
}
} // namespace Aether::Audio