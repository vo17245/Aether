#include "Mp3Test.h"
#include "Aether/Core/Log.h"
#include "Aether/ImGui/imgui.h"
#include "Test/Mp3Test.h"
#include "Test/TestRegister.h"
#include "TestRegister.h"
#include "minimp3.h"
#include "minimp3_ex.h"
#include <samplerate.h>
namespace Aether
{
    namespace Test
    {

        REGISTER_TEST(Mp3Test)

       
struct PcmData
{
    std::byte* data;
    uint8_t channels;
    uint8_t depth;
    uint32_t sample_count;
    uint32_t frequency;
    uint32_t calculate_size()
    {
        return sample_count * channels * depth / 8;
    } 
};

static void ResampleAudio(PcmData& input,PcmData& output)
{
    std::vector<float> inputf(input.channels*input.sample_count);
    if(input.depth==8)
    {
        for(size_t i=0;i<input.sample_count;i++)
        {
            for(size_t j=0;j<input.channels;j++)
            {
                inputf[i*input.channels+j]=static_cast<float>(static_cast<int8_t>(input.data[i*input.channels+j]))/128.0f;
            }
        }
    }
    else if(input.depth==16)
    {
        for(size_t i=0;i<input.sample_count;i++)
        {
            for(size_t j=0;j<input.channels;j++)
            {
                inputf[i*input.channels+j]=static_cast<float>(static_cast<int16_t>(input.data[i*input.channels+j]))/32768.0f;
            }
        }
    }
    else if(input.depth==32)
    {
        for(size_t i=0;i<input.sample_count;i++)
        {
            for(size_t j=0;j<input.channels;j++)
            {
                inputf[i*input.channels+j]=static_cast<float>(static_cast<int32_t>(input.data[i*input.channels+j]))/2147483648.0f;
            }
        }
    }
    else
    {
        AETHER_DEBUG_LOG_ERROR("unsupported depth");
        return;
    }
    std::vector<float> outputf(output.channels*output.sample_count);

    int error;
    SRC_STATE* src = src_new(SRC_SINC_FASTEST, input.channels, &error);
    if (src == nullptr)
    {
        src_delete(src);
        AETHER_DEBUG_LOG_ERROR("failed to create resampler");
        return;
    }

    SRC_DATA data;
    data.data_in = inputf.data();
    data.input_frames = input.sample_count;
    data.data_out = outputf.data();
    data.output_frames = output.sample_count;
    data.src_ratio = static_cast<double>(output.frequency) / input.frequency;

    error = src_process(src, &data);
    if (error)
    {
        // handle error
        AETHER_DEBUG_LOG_ERROR("failed to resample");
        src_delete(src);
        return;
    }

    src_delete(src);

    // Convert float to int
    if(output.depth==8)
    {
        for(size_t i=0;i<output.sample_count;i++)
        {
            for(size_t j=0;j<output.channels;j++)
            {
                output.data[i*output.channels+j]=static_cast<std::byte>(outputf[i*output.channels+j]*128.0f);
            }
        }
    }
    else if(output.depth==16)
    {
        for(size_t i=0;i<output.sample_count;i++)
        {
            for(size_t j=0;j<output.channels;j++)
            {
                output.data[i*output.channels+j]=static_cast<std::byte>(outputf[i*output.channels+j]*32768.0f);
            }
        }
    }
    else if(output.depth==32)
    {
        for(size_t i=0;i<output.sample_count;i++)
        {
            for(size_t j=0;j<output.channels;j++)
            {
                output.data[i*output.channels+j]=static_cast<std::byte>(outputf[i*output.channels+j]*2147483648.0f);
            }
        }
    }
    else
    {
        AETHER_DEBUG_LOG_ERROR("unsupported depth");
        return;
    
    }
    
}
        void Mp3Test::OnImGuiRender()
        {
            ImGui::Begin("Mp3 Test");
            bool play=ImGui::Button("play");
            ImGui::End();

            static std::string path="../../Asset/Music/48khz_Cody Matthew Johnson - Subhuman.mp3";
           
            if(play)
            {
                //load mp3
                mp3dec_ex_t dec;
                if (mp3dec_ex_open(&dec, path.c_str(), MP3D_SEEK_TO_SAMPLE))
                {
                    /* error */
                    AETHER_DEBUG_LOG_ERROR("failed to open mp3 file");
                }
                
                mp3d_sample_t *buffer = (mp3d_sample_t *)malloc(dec.samples*sizeof(mp3d_sample_t));
                size_t readed = mp3dec_ex_read(&dec, buffer, dec.samples);
                if (readed != dec.samples) /* normal eof or error condition */
                {
                    if (dec.last_error)
                    {
                        /* error */
                        AETHER_DEBUG_LOG_ERROR("failed to load mp3 file");
                    }
                }

                int channels = dec.info.channels;
                int depth = 16;
                int freq = dec.info.hz;
                int frames = dec.samples/channels;
                //play
                auto device = AudioApi::GetDefaultDevice();
                AudioApi::PlayPCMSync(device, (const char*)buffer, frames,  depth,  freq,  channels);
                AudioApi::ReleaseDevice(device);
                
                free(buffer);
                mp3dec_ex_close(&dec);

            }
        }
    }
}