#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <format>
namespace Aether {
// float32
using Vec2f = Eigen::Vector2f;
using Vec3f = Eigen::Vector3f;
using Vec4f = Eigen::Vector4f;
using Mat2f = Eigen::Matrix2f;
using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;
// float32 alias
using Vec2 = Vec2f;
using Vec3 = Vec3f;
using Vec4 = Vec4f;
using Mat2 = Mat2f;
using Mat3 = Mat3f;
using Mat4 = Mat4f;

// double
using Vec2d = Eigen::Vector2d;
using Vec3d = Eigen::Vector3d;
using Vec4d = Eigen::Vector4d;
using Mat2d = Eigen::Matrix2d;
using Mat3d = Eigen::Matrix3d;
using Mat4d = Eigen::Matrix4d;

// int32 
using Vec2i = Eigen::Vector2<int32_t>;
using Vec3i = Eigen::Vector3<int32_t>;
using Vec4i = Eigen::Vector4<int32_t>;
using Mat2i = Eigen::Matrix2<int32_t>;
using Mat3i = Eigen::Matrix3<int32_t>;
using Mat4i = Eigen::Matrix4<int32_t>;

// uint32 
using Vec2u = Eigen::Vector2<uint32_t>;
using Vec3u = Eigen::Vector3<uint32_t>;
using Vec4u = Eigen::Vector4<uint32_t>;
using Mat2u = Eigen::Matrix2<uint32_t>;
using Mat3u = Eigen::Matrix3<uint32_t>;
using Mat4u = Eigen::Matrix4<uint32_t>;

// Quaterniond
using Quatd = Eigen::Quaterniond;
using Quat = Eigen::Quaternionf;

namespace Math {
inline constexpr float PI = 3.14159265358979323846f;
}

} // namespace Aether

template <>
struct std::formatter<Aether::Vec2f>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.end();
    }
    template <class FmtContext>
    FmtContext::iterator format(Aether::Vec2f& v, FmtContext& ctx) const
    {
        return format_to(ctx.out(), "({}, {})", v.x(), v.y());
    }
};

template <>
struct std::formatter<Aether::Vec3f>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.end();
    }
    template <class FmtContext>
    FmtContext::iterator format(const Aether::Vec3f& v, FmtContext& ctx) const
    {
        return format_to(ctx.out(), "({}, {}, {})", v.x(), v.y(), v.z());
    }
};

template <>
struct std::formatter<Aether::Vec4f>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.end();
    }
    template <class FmtContext>
    FmtContext::iterator format(const Aether::Vec4f& v, FmtContext& ctx) const
    {
        return format_to(ctx.out(), "({}, {}, {}, {})", v.x(), v.y(), v.z(), v.w());
    }
};

template <>
struct std::formatter<Aether::Mat4f>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.end();
    }
    // auto format(const Aether::Mat4f& m, format_context& ctx) const
    //{
    //     return format_to(ctx.out(), "({},\n {},\n {},\n {})",
    //                      static_cast<Aether::Vec4f>(m.row(0)),
    //                      static_cast<Aether::Vec4f>(m.row(1)),
    //                      static_cast<Aether::Vec4f>(m.row(2)),
    //                      static_cast<Aether::Vec4f>(m.row(3)));
    // }
    template <class FmtContext>
    FmtContext::iterator format(const Aether::Mat4f& m, FmtContext& ctx) const
    {
        return format_to(ctx.out(),
                         R"({},{},{},{},
{},{},{},{},
{},{},{},{},
{},{},{},{})",
                         m(0, 0), m(0, 1), m(0, 2), m(0, 3),
                         m(1, 0), m(1, 1), m(1, 2), m(1, 3),
                         m(2, 0), m(2, 1), m(2, 2), m(2, 3),
                         m(3, 0), m(3, 1), m(3, 2), m(3, 3));
    }
};
template <>
struct std::formatter<Aether::Quat>
{
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.end();
    }
  
    template <class FmtContext>
    FmtContext::iterator format(const Aether::Quat& q, FmtContext& ctx) const
    {
        return format_to(ctx.out(), "({}, {}, {}, {})", q.w(), q.x(), q.y(), q.z());
    }
};
