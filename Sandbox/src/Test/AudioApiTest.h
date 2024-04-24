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
            }
            virtual ~AudioApiTest()
            {
            }
            void OnImGuiRender()override;
        };
    }
}