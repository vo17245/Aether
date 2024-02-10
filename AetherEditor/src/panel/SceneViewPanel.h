#pragma once
#include "Aether/Render/FrameBuffer.h"
#include "Aether/Resource/Model/Model.h"
namespace Aether
{
class SceneViewPanel
{
public:
    SceneViewPanel(size_t width,size_t height);
private:
    Ref<FrameBuffer> m_HdrFrameBuffer;
    Ref<Model> m_ScreenQuad;
};
}