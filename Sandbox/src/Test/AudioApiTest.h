#pragma once
#include "Test.h"
#include "Aether/Audio.h"

namespace Aether
{
    namespace Test
    {
        class AudioApiTest:public Test
        {
        public:
            AudioApiTest()
            {
                Aether::AudioApi::Init();
            }
            virtual ~AudioApiTest()
            {
                Aether::AudioApi::Shutdown();
            }
            void OnImGuiRender()override;
        };
    }
}