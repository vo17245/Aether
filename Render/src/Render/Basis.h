#pragma once
#include "Config.h"
#include "Core/Math.h"
#include "Render/Config.h"
#include <cassert>
namespace Aether
{
    struct Basis3D
    {
        Vec3f o;
        Vec3f x;
        Vec3f y;
        Vec3f z;
    };
    struct Basis2D
    {
        Vec2f o;
        Vec2f x;
        Vec2f y;
    };
    /**
    * @brief 在opengl ndc坐标上描述basis
    *     ^ y
    *     |
    *     |
    *     |
    *     +----->x
    *    /
    *   /
    * \/_
    *    z   
    */
    inline Basis3D GetNdcBasis()
    {
        if(Render::Config::RenderApi==Render::Api::Vulkan)
        {
            /**
            * @note z range is [0,1]
            */
            return Basis3D{Vec3f(0,0,0),Vec3f(1,0,0),Vec3f(0,-1,0),Vec3f(0,0,1)};
        }
        else {
            assert(false&&"Not implemented");
            return Basis3D{Vec3f(0,0,0),Vec3f(1,0,0),Vec3f(0,-1,0),Vec3f(0,0,1)};
        }
    }

    /**
    * @brief 在opengl texture坐标上描述basis
    *        ^ y
    *        |
    *        |
    *        |
    *        +----->x
    */
    inline Basis2D GetTexCoordBasis()
    {
        if(Render::Config::RenderApi==Render::Api::Vulkan)
        {
            return Basis2D{Vec2f(0,1),Vec2f(1,0),Vec2f(0,-1)};
        }
        else {
            assert(false&&"Not implemented");
        }
    }
    /**
     * @brief 在opengl ndc上描述basis
     * @note 因为在opengl ndc的描述上屏幕坐标进行了伸缩，所以basis的方向向量的长度不是1(是2)
    */
    inline Basis3D GetNormalizedScreenBasis()
    {
        return Basis3D{Vec3f(-1,1,0),Vec3f(2,0,0),Vec3f(0,-2,0),Vec3f(0,0,2)};
    }
    /**
     * @note 如果basis不是正交的，可能会抛异常
     *      如果basis有伸缩变换，通过改变basis的方向向量的长度来表达
     * @return 把点在basis from 下的表示转换为basis to 下的表示
    */
    inline Mat3 CalculateBasisTransform(const Basis2D& from,const Basis2D& to)
    {
        Mat2 e_from=Mat2::Identity();
        e_from.block<2,1>(0,0)=from.x;
        e_from.block<2,1>(0,1)=from.y;
        Mat2 e_to=Mat2::Identity();
        e_to.block<2,1>(0,0)=to.x;
        e_to.block<2,1>(0,1)=to.y;
        Mat2 e_to_inv=e_to.inverse();
        Vec2f oto_ofrom=from.o-to.o;
        Mat2 m=e_to_inv*e_from;
        Vec2 v=e_to_inv*oto_ofrom;
        Mat3 res = Math::MergeTranslation(m, v);
        return res;
    }
    inline Mat4 CalculateBasisTransform(const Basis3D& from,const Basis3D& to)
    {
        Mat3 e_from=Mat3::Identity();
        e_from.block<3,1>(0,0)=from.x;
        e_from.block<3,1>(0,1)=from.y;
        e_from.block<3,1>(0,2)=from.z;
        Mat3 e_to=Mat3::Identity();
        e_to.block<3,1>(0,0)=to.x;
        e_to.block<3,1>(0,1)=to.y;
        e_to.block<3,1>(0,2)=to.z;
        Mat3 e_to_inv=e_to.inverse();
        Vec3f oto_ofrom=from.o-to.o;
        Mat3 m=e_to_inv*e_from;
        Vec3f v=e_to_inv*oto_ofrom;
        return Math::MergeTranslation(m,v);
    }
}