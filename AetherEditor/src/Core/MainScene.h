#pragma once
#include "Aether/Scene.h"
#include "Aether/Scene/Scene.h"
namespace Aether
{
    namespace Editor
    {
        class MainScene
        {
        public:
            ~MainScene();
            static MainScene& GetInstance()
            {
                static MainScene instance;
                return instance;
            }
            Scene& GetScene()
            {
                return m_Scene;
            }
        private:
            MainScene();
            Scene m_Scene;

        };
    }
}