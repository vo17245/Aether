#pragma once
#include "Render/PixelFormat.h"
#include "Render/RenderApi/DeviceFrameBuffer.h"
#include "Render/RenderApi/DeviceTexture.h"
using namespace Aether;
class ApplicationResource
{
public:
    static std::optional<std::string> Init()
    {
        s_Instance = new ApplicationResource();

        return s_Instance->InitImpl();

    }
    static void Destroy()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
    ~ApplicationResource()
    {
       
    }
    ApplicationResource& GetSingleton()
    {
        return *s_Instance;
    }

private:
    std::optional<std::string> InitImpl()
    {
        return std::nullopt;
    }
    static ApplicationResource* s_Instance;
   
};
