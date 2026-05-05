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
    };
    struct DepthAttachment
    {
        TextureView* view;
        AttachmentLoadOp loadOp;
        AttachmentStoreOp storeOp;
    };
}