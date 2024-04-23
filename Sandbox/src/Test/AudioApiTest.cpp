#include "AudioApiTest.h"
#include "Aether/ImGui/imgui.h"
#include "Test/TestRegister.h"
#include "Aether/Core/Log.h"
#include <cmath>
#include <stdint.h>
#include <string>
#include <unordered_map>

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
            static auto freq=std::unordered_map<std::string, double>({
            {"C4", 261.63},
            {"D4", 293.66},
            {"E4", 329.63},
            {"F4", 349.23},
            {"G4", 392.00},
            {"A4", 440.00},
            {"B4", 493.88},
            {"C5", 523.25}
            });
            static auto duration=std::unordered_map<std::string, double>({
            {"whole", 1.0},
            {"half", 0.5},
            {"quarter", 0.25},
            {"eighth", 0.125},
            {"sixteenth", 0.0625}
            });
            static auto melody=std::vector<std::pair<std::string,std::string>>({
            {"C4", "quarter"},
            {"D4", "eighth"},
            {"E4", "eighth"},
            {"C4", "quarter"},
            {"C4", "eighth"},
            {"D4", "eighth"},
            {"E4", "eighth"},
            {"E4", "quarter"},
            {"F4", "eighth"},
            {"G4", "eighth"},
            {"C4", "quarter"},
            {"C4", "eighth"},
            {"G4", "eighth"},
            {"F4", "eighth"},
            {"E4", "half"},
            {"C4", "quarter"},
            {"D4", "eighth"},
            {"E4", "eighth"},
            {"C4", "quarter"},
            {"C4", "eighth"},
            {"D4", "eighth"},
            {"E4", "eighth"},
            {"E4", "quarter"},
            {"F4", "eighth"},
            {"G4", "eighth"},
            {"C4", "quarter"},
            {"C4", "eighth"},
            {"G4", "eighth"},
            {"F4", "eighth"},
            {"E4", "half"},
            {"C4", "quarter"},
            {"C5", "eighth"},
            {"C5", "eighth"},
            {"C5", "quarter"},
            {"B4", "eighth"},
            {"B4", "eighth"},
            {"B4", "quarter"},
            {"A4", "eighth"},
            {"A4", "eighth"},
            {"A4", "quarter"},
            {"G4", "half"},
            });
            if(is_play)
            {
                float sec_f=0;
                for(auto [f,d]:melody)
                {
                    sec_f+=duration[d];
                }
                AETHER_DEBUG_LOG("play begin");
                auto device = AudioApi::GetDefaultDevice();
                int sec=std::floor(sec_f)+1;;
                int depth=16;
                int channels=2;
                int frequency=48000;
                int16_t* pcmData=new int16_t[sec*frequency*channels];
                size_t offset=0;
                for(auto [f,d]:melody)
                {
                    int frame_count=duration[d]*frequency;
                    double cur_f=freq[f];
                    for(int i=offset;i<frame_count+offset;i++)
                    {
                        double t=static_cast<double>(i)/frequency;
                        double v=std::sin(2*3.14159265358979323846*cur_f*t);
                        pcmData[i*channels]=static_cast<int16_t>(v*32767)*(frame_count-i+offset-1)/frame_count*(1-(frame_count-i+offset-1)/frame_count);
                        pcmData[i*channels+1]=static_cast<int16_t>(v*32767)*(frame_count-i+offset)/frame_count*(1-(frame_count-i+offset)/frame_count);
                    }
                    offset+=frame_count;
                }
                AudioApi::PlayPCMSync(device, (const char*)pcmData, sec*frequency,  depth,  frequency,  channels);
                delete[] pcmData;
                AudioApi::ReleaseDevice(device);
                AETHER_DEBUG_LOG("play end");
            }
        }
    }
}