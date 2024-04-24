#pragma once
#include "Test.h"
namespace Aether
{
    namespace Test
    {
        class AudioServerTest:public Test
        {
        public:
            AudioServerTest()=default;
            ~AudioServerTest()=default;
            void OnImGuiRender()override;
        };
    }
}