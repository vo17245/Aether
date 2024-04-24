#include "AudioServerTest.h"
#include "TestRegister.h"
#include "Aether/Server.h"
namespace Aether
{
    namespace Test
    {
        REGISTER_TEST(AudioServerTest);
        void AudioServerTest::OnImGuiRender()
        {
            ImGui::Begin("AudioServerTest");
            bool play_48khz=ImGui::Button("play 48khz");
            ImGui::End();
            static std::string path_48khz="../../Asset/Music/48khz_Cody Matthew Johnson - Subhuman.mp3";
            if(play_48khz)
            {
                
                AudioServer::PlayMp3File(path_48khz);
            }
        }
    }
}