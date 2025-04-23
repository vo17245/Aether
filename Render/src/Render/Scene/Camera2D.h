#pragma once
#include "Core/Math.h"
#include "../Basis.h"
namespace Aether
{
struct Camera2D
{
    Vec2f offset={0, 0};
    Vec2f target={0, 0};
    float zoom=1.0f;
    float rotation=0.0f;
    Vec2f screenSize={0, 0};
    Mat4f matrix=Mat4f::Identity();
    void CalculateMatrix()
    {
        // to camera space (view)
        // 把target移动到中心
        Mat4f m = Math::Translation(Vec3f(-target.x(), -target.y(), 0)) ;
        // 旋转
        m = Math::RotateZ(rotation) * m;
        // offset
        m = Math::Translation(Vec3f(offset.x(), offset.y(), 0)) * m;
        // 缩放
        m = Math::Scale(Vec3f(zoom, zoom, 1)) * m;
        //==========
        // to NDC space (projection)
        // screen to [0,1]
        m=Math::Scale(Vec3f(1/screenSize.x(), 1/screenSize.y(), 1))*m;
        // screen to NDC
        m=CalculateBasisTransform(GetNormalizedScreenBasis(), GetNdcBasis())*m;
        //=============
        matrix = m;
    }
};
} // namespace Aether