#include "AudioServer.h"
#include "Aether/Audio/AudioApi.h"
#include <functional>
#include <mutex>
#include "Aether/Server/AudioServer/AudioServer.h"
#include "minimp3.h"
#include "minimp3_ex.h"


namespace Aether
{
    bool AudioServer::Init()
    {
        m_AudioDevice=AudioApi::GetDefaultDevice();
        if(m_AudioDevice.ptr==nullptr)
        {
            return false;
        }
        m_Running=true;
        m_WorkerThread=new std::thread([this](){WorkerThreadFunc();});

    }
    void AudioServer::Shutdown()
    {
        m_Running=false;
        m_WorkerThread->join();
        delete m_WorkerThread;
        AudioApi::ReleaseDevice(m_AudioDevice);
    }

    
    
    void AudioServer::WorkerThreadFunc()
    {
        
        AudioServerCommand::Command cmd;
        while(true)
        {
            {
               std::unique_lock<std::mutex> lock(m_Mutex);
               m_Condition.wait(lock,[this](){return !m_CommandBuffer.IsEmpty()||!m_Running;});
               if(!m_Running&&m_CommandBuffer.IsEmpty())
               {
                   break;
               }
               m_CommandBuffer.Read(&cmd,1);
            }
            std::visit(Executor(m_AudioDevice),cmd);

            
        }

    }


    void AudioServer::Executor::operator()(const AudioServerCommand::PlayMp3FileCommand& cmd)
    {
        auto& path=cmd.path;
        //load mp3
        mp3dec_ex_t dec;
        if (mp3dec_ex_open(&dec, path.c_str(), MP3D_SEEK_TO_SAMPLE))
        {
            /* error */
            AETHER_DEBUG_LOG_ERROR("failed to open mp3 file");
            return;
        }
        mp3d_sample_t *buffer = (mp3d_sample_t *)malloc(dec.samples*sizeof(mp3d_sample_t));
        size_t readed = mp3dec_ex_read(&dec, buffer, dec.samples);
        if (readed != dec.samples) /* normal eof or error condition */
        {
            if (dec.last_error)
            {
                /* error */
                AETHER_DEBUG_LOG_ERROR("failed to load mp3 file");
                free(buffer);
                return;
            }
        }
        int channels = dec.info.channels;
        int depth = 16;
        int freq = dec.info.hz;
        int frames = dec.samples/channels;
        //play
        AudioApi::PlayPCMSync(device, (const char*)buffer, frames,  depth,freq,   channels);
        free(buffer);
        mp3dec_ex_close(&dec);
    }
}//namespace Aether
