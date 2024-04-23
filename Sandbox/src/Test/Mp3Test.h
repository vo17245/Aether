#pragma once
#include "Aether/Audio/AudioApi.h"
#include "minimp3.h"
#include "Test.h"
#include "Aether/Audio.h"
namespace Aether
{
    namespace Test
    {
        class Mp3Test:public Test
        {
        public:
            Mp3Test(){AudioApi::Init();}
            ~Mp3Test(){AudioApi::Shutdown();}
            void OnImGuiRender()override;
        };
    }
    
}