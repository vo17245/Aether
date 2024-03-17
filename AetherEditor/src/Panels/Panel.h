#pragma once

namespace Aether
{
    namespace Editor
    {
        class Panel
        {
        public:
            Panel()=default;
            virtual ~Panel()=default;
            virtual void OnImGuiRender(){};
            virtual void OnRender(){};
        };
    }
}