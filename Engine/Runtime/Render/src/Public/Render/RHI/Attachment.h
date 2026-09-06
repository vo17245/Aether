#pragma once
#include "TextureView.h"

namespace Aether::rhi
{
    enum class AttachmentLoadOp
    {
        Load,
        Clear,
        DontCare
    };
    enum class AttachmentStoreOp
    {
        Store,
        DontCare
    };
    struct ColorAttachment
    {
        TextureView* view;
        AttachmentLoadOp loadOp;
        AttachmentStoreOp storeOp;
        Vec4f clearColor = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    };
    struct DepthAttachment
    {
        TextureView* view;
        AttachmentLoadOp loadOp;
        AttachmentStoreOp storeOp;
        float clearDepth = 1.0f;
        uint32_t clearStencil = 0;
    };
}