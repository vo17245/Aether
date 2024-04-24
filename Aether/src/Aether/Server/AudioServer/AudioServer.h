#pragma once
#include <mutex>
#include <string>
#include "Aether/Core.h"
#include "Aether/Core/Application.h"
#include <string_view>
#include <thread>
#include <variant>
#include "Aether/Audio.h"
#include "AudioServerCommand.h"

namespace Aether
{
    
    class Application;
    class AudioServer
    {
        friend class Application;
    public:
        constexpr const inline static size_t COMMAND_BUFFER_CAPACITY=1024;
        
    public://commit command interface
        static bool PlayMp3File(const std::string& path)
        {
            return GetInstance().PlayMp3FileImpl(path);
        }
        
    private://singleton
        AudioServer():m_CommandBuffer(COMMAND_BUFFER_CAPACITY){}
        ~AudioServer()=default;
        static AudioServer& GetInstance()
        {
            static AudioServer instance;
            return instance;
        }

    private://commit command 
        
        bool PlayMp3FileImpl(const std::string& path)
        {
            AudioServerCommand::Command cmd=AudioServerCommand::PlayMp3FileCommand(path);
            return CommitCommandImpl(cmd);
        }
        bool CommitCommandImpl(const AudioServerCommand::Command& cmd)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            bool res=m_CommandBuffer.Write(&cmd,1)==1;
            if(res)
            {
                m_Condition.notify_one();
            }
            return res;
        }
    private://worker thread
        RingBuffer<AudioServerCommand::Command> m_CommandBuffer;
        void WorkerThreadFunc();
        bool m_Running;
        std::thread* m_WorkerThread;
        bool Init();
        void Shutdown();
        std::mutex m_Mutex;
        std::condition_variable m_Condition;
    private://audio device
        AudioDeviceH m_AudioDevice;
    private://command executor
        struct Executor
        {
            Executor(AudioDeviceH _device):device(_device){}
            AudioDeviceH device;
            void operator()(const std::monostate&){}//do nothing
            void operator()(const AudioServerCommand::PlayMp3FileCommand& cmd);
            
        };

    };
}//namespace Aether
