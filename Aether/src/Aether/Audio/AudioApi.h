#pragma once
#include <string>

namespace Aether
{
    struct AudioDeviceH{void* ptr;};
    class AudioApi
    {
    public:
        static void Init();
        static void Shutdown();
        static AudioDeviceH GetDefaultDevice();
        static void ReleaseDevice(AudioDeviceH device);
        static void PlayPCMSync(AudioDeviceH device, const char* pcmData, size_t size, int depth, int frequency,int channels);
    };
}