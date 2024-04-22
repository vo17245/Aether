#include "AudioApiTest.h"
#include "Aether/ImGui/imgui.h"
#include "Test/TestRegister.h"
#include "Aether/Core/Log.h"
#include <stdint.h>

namespace Aether
{
    namespace Test
    {
        
        REGISTER_TEST(AudioApiTest);
        void AudioApiTest::OnImGuiRender()
        {
            ImGui::Begin("AudioApiTest");
            bool is_play=ImGui::Button("play");

            ImGui::End();
            if(is_play)
            {
                AETHER_DEBUG_LOG("play begin");
                auto device = AudioApi::GetDefaultDevice();
                int sec=10;
                int depth=32;
                int channels=2;
                int frequency=48000;
                uint16_t* pcmData=new uint16_t[sec*frequency*channels];
                for(size_t i=0;i<sec*frequency*channels;i++)
                {
                    pcmData[i]=32767*sin(2*3.1415926*440*i/frequency);
                }
                AudioApi::PlayPCMSync(device, (const char*)pcmData, sec*frequency*channels,  depth,  frequency,  channels);
                delete[] pcmData;
                AudioApi::ReleaseDevice(device);
                AETHER_DEBUG_LOG("play end");
            }
        }
    }
}