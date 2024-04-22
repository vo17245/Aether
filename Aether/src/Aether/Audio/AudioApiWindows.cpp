#include "AudioApi.h"
#include "Aether/Core.h"
#include <mmeapi.h>


//============ windows implmentation ============
#ifdef AETHER_PLATFORM_WINDOWS
#include <Audioclient.h>
#include <mmdeviceapi.h>

namespace Aether
{
    static std::string WaveFormat2String(WAVEFORMATEX waveFormat)
    {
        return "wFormatTag:"+std::to_string(waveFormat.wFormatTag)+
        " nChannels:"+std::to_string(waveFormat.nChannels)+
        " nSamplesPerSec:"+std::to_string(waveFormat.nSamplesPerSec)+
        " wBitsPerSample:"+std::to_string(waveFormat.wBitsPerSample)+
        " nBlockAlign:"+std::to_string(waveFormat.nBlockAlign)+
        " nAvgBytesPerSec:"+std::to_string(waveFormat.nAvgBytesPerSec);
    }
    
    void AudioApi::Init()
    {
        // Initialize the COM library
        CoInitialize(NULL);
    }
    void AudioApi::Shutdown()
    {
        CoUninitialize();
    }
    AudioDeviceH AudioApi::GetDefaultDevice()
    {
       

        // Create device enumerator
        IMMDeviceEnumerator* deviceEnumerator = nullptr;
        CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);

        // Get default audio endpoint
        IMMDevice* audioDevice = nullptr;
        deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audioDevice);

        // Create an AudioDevice instance from the IMMDevice instance
        AudioDeviceH defaultDevice = {(void*)audioDevice};

        // Clean up
        //audioDevice->Release();
        deviceEnumerator->Release();
        

        return defaultDevice;
    }
    
    void AudioApi::PlayPCMSync(AudioDeviceH device, const char *pcmData, size_t size, int depth, int frequency,int channels)
    {
    HRESULT hr;
    IMMDevice* audioDevice=(IMMDevice*)device.ptr;
    // Create the audio client
    IAudioClient* audioClient = nullptr;
    audioDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);

    // Set the audio stream format
    WAVEFORMATEX waveFormat = {0};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = channels;
    waveFormat.nSamplesPerSec = frequency;
    waveFormat.wBitsPerSample = depth;
    waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    //is waveformat supported?
    WAVEFORMATEX* closestMatch = nullptr;
    hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, & waveFormat, &closestMatch);
    if (hr == S_FALSE)
    {
        // The requested format is not supported, but closestMatch contains a suggested format
        // Use closestMatch to initialize the audio client
        
        AETHER_DEBUG_LOG_WARN("wave format change: \n"
        "from \n{}\nto\n{}\n",WaveFormat2String(waveFormat),WaveFormat2String(*closestMatch));

        hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, closestMatch, nullptr);
        CoTaskMemFree(closestMatch);
    }
    else if (hr == S_OK)
    {
        // The requested format is supported
        // Continue with the current waveFormat
        hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, &waveFormat, nullptr);
    }
    else
    {
        //TODO: handle error
        // An error occurred
        // Handle error
        AETHER_DEBUG_LOG_ERROR("Failed to initialize audio client,format not supported");
        return;
    }

    //hr=audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, &waveFormat, nullptr);
    if (FAILED(hr))
    {
        std::string msg="unknown error";
        switch(hr)
        {
            case AUDCLNT_E_ALREADY_INITIALIZED:
                msg="The IAudioClient object is already initialized";
                break;
            case AUDCLNT_E_WRONG_ENDPOINT_TYPE:
                msg="The AUDCLNT_SHAREMODE_SHARED mode is not supported by the device";
                break;
            case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:
                msg="The requested buffer size is not aligned";
                break;
            case AUDCLNT_E_UNSUPPORTED_FORMAT:
                msg="The specified wave format is not supported"; 
                break;
            // Add more cases as needed
        }
        AETHER_DEBUG_LOG_ERROR("Failed to initialize audio client,{}",msg);
        return;
    }
    // Get the audio render client
    IAudioRenderClient* audioRenderClient = nullptr;
    hr=audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&audioRenderClient);
    if (FAILED(hr) || audioRenderClient == nullptr)
    {
        std::string msg="unknown error";
        switch(hr)
        {
            case E_POINTER:
                msg= "audioRenderClient is not a valid pointer";
                break;
            case E_NOINTERFACE:
                msg= "The requested interface is not available";
                break;
            case AUDCLNT_E_NOT_INITIALIZED:
                msg= "The audioClient has not been initialized";
                break;
            // Add more cases as needed
        }
        AETHER_DEBUG_LOG_ERROR("Failed to get audio render client,{}",msg);
        return;
    }

    // Start playing
    audioClient->Start();
    // upload pcm data
    UINT32 totalFramesToWrite = size / (waveFormat.nChannels * waveFormat.wBitsPerSample / 8);
UINT32 framesWritten = 0;

while (framesWritten < totalFramesToWrite)
{
    UINT32 bufferFrameCount;
    hr=audioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr))
    {
        std::string msg="unknown error";
        switch(hr)
        {
            case E_POINTER:
                msg="The pNumBufferFrames parameter is NULL"; 
                break;
            case AUDCLNT_E_NOT_INITIALIZED:
                msg="The audioClient has not been initialized"; 
                break;
            // Add more cases as needed
        }
        AETHER_DEBUG_LOG_ERROR("Failed to get buffer size,{}",msg);
        return;
    }
    UINT32 padding = 0;
    hr = audioClient->GetCurrentPadding(&padding);
    if (FAILED(hr))
    {
        // Handle error
        return;
    }
    UINT32 availableSpace = bufferFrameCount - padding;
    // Calculate the number of frames to write in this iteration
    UINT32 framesToWrite = std::min(availableSpace, totalFramesToWrite - framesWritten);

    // Get the audio buffer
    BYTE* audioData = nullptr;
    hr = audioRenderClient->GetBuffer(framesToWrite, &audioData);
    if (FAILED(hr))
    {
        std::string msg = "unknown error";
        switch(hr)
        {
            case AUDCLNT_E_OUT_OF_ORDER:
                msg = "The previous IAudioRenderClient::ReleaseBuffer call reported an  error, or the client is not initialized.";
                break;
            case AUDCLNT_E_DEVICE_INVALIDATED:
                msg = "The audio device has been unplugged, or the audio hardware or    associated hardware resources have been reconfigured, disabled,    removed, or otherwise made unavailable for use.";
                break;
            case AUDCLNT_E_BUFFER_TOO_LARGE:
                msg = "The requested size of the buffer is larger than the maximum size     for the buffer.";
                break;
            // Add more cases as needed
        }
        AETHER_DEBUG_LOG_ERROR("Failed to get buffer, {}", msg);
        return;
    }

    // Copy the data to the audio buffer
    memcpy(audioData, pcmData + framesWritten * waveFormat.nChannels * waveFormat.wBitsPerSample / 8, framesToWrite * waveFormat.nChannels * waveFormat.wBitsPerSample / 8);

    // Release the audio buffer
    hr = audioRenderClient->ReleaseBuffer(framesToWrite, 0);
    if (FAILED(hr))
    {
        // Handle error
        return;
    }

    // Update the number of frames written
    framesWritten += framesToWrite;


    
    // Wait for the audio to finish playing
    //padding = 0;
    //do
    //{
    //    Sleep(100);  // Wait for a short time
    //    hr = audioClient->GetCurrentPadding(&padding);
    //    if (FAILED(hr))
    //    {
    //        // Handle error
    //        break;
    //    }
    //} while (padding > 0);
    
    
}
// Clean up
audioClient->Stop();
audioRenderClient->Release();
audioClient->Release();
    // Get the audio buffer
    //UINT32 bufferFrameCount;
    //hr=audioClient->GetBufferSize(&bufferFrameCount);
    //if (FAILED(hr))
    //{
    //    std::string msg="unknown error";
    //    switch(hr)
    //    {
    //        case E_POINTER:
    //            msg="The pNumBufferFrames parameter is NULL"; 
    //            break;
    //        case AUDCLNT_E_NOT_INITIALIZED:
    //            msg="The audioClient has not been initialized"; 
    //            break;
    //        // Add more cases as needed
    //    }
    //    AETHER_DEBUG_LOG_ERROR("Failed to get buffer size,{}",msg);
    //    return;
    //}
    //BYTE* audioData = nullptr;
    //hr=audioRenderClient->GetBuffer(bufferFrameCount, &//audioData);
    //if (FAILED(hr))
    //{
    //    std::string msg="unknown error";
    //    switch(hr)
    //    {
    //        case E_POINTER:
    //            msg="The pData parameter is NULL"; 
    //            break;
    //        case AUDCLNT_E_OUT_OF_ORDER:
    //            msg="The method was called in the wrong order"; 
    //            break;
    //        case AUDCLNT_E_BUFFER_ERROR:
    //            msg="An error occurred"; 
    //            break;
    //        // Add more cases as needed
    //    }
    //    AETHER_DEBUG_LOG_ERROR("Failed to get buffer,{}",msg);
    //    return;
    //}
    //// Copy the PCM data to the audio buffer
    ////memcpy(audioData, pcmData, size);
    //memcpy(audioData, pcmData, bufferFrameCount * waveFormat.//nBlockAlign);
    //// Release the audio buffer
    //hr=audioRenderClient->ReleaseBuffer(bufferFrameCount, //0);
    //if (FAILED(hr))
    //{
    //    std::string msg="unknown error";
    //    switch(hr)
    //    {
    //        case AUDCLNT_E_BUFFER_TOO_LARGE:
    //            msg="The pNumFramesWritten parameter is larger than the padding in //the buffer";
    //            break;
    //        case AUDCLNT_E_OUT_OF_ORDER:
    //            msg="The call to ReleaseBuffer was not preceded by a call to //GetBuffer for the same buffer"; 
    //            break;
    //        case AUDCLNT_E_BUFFER_ERROR:
    //            msg="An error occurred"; 
    //            break;
    //        // Add more cases as needed
    //    }
    //    AETHER_DEBUG_LOG_ERROR("Failed to release buffer,{}",msg);
    //}
    
}
void AudioApi::ReleaseDevice(AudioDeviceH device)
{
    IMMDevice* audioDevice = (IMMDevice*)device.ptr;
    audioDevice->Release();
}
}//namespace Aether



#endif